/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.commserver;

import static org.springframework.dao.support.DataAccessUtils.intResult;
import static org.springframework.dao.support.DataAccessUtils.singleResult;

import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.Criteria;
import org.hibernate.Session;
import org.hibernate.criterion.Criterion;
import org.hibernate.criterion.Restrictions;
import org.sipfoundry.sipxconfig.commserver.imdb.ReplicationManager;
import org.sipfoundry.sipxconfig.dialplan.config.ConfigGenerator;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventPublisher;
import org.sipfoundry.sipxconfig.domain.DomainConfiguration;
import org.sipfoundry.sipxconfig.nattraversal.NatLocation;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversalManager;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.springframework.orm.hibernate3.HibernateCallback;

public abstract class LocationsManagerImpl extends SipxHibernateDaoSupport<Location> implements LocationsManager {
    public static final Log LOG = LogFactory.getLog(LocationsManagerImpl.class);
    private static final String LOCATION_PROP_NAME = "fqdn";
    private static final String LOCATION_PROP_PRIMARY = "primary";
    private static final String LOCATION_PROP_IP = "ipAddress";
    private static final String LOCATION_PROP_ID = "locationId";
    private static final String DUPLICATE_FQDN_OR_IP = "&error.duplicateFqdnOrIp";
    private DaoEventPublisher m_daoEventPublisher;

    protected abstract NatTraversalManager getNatTraversalManager();
    /* delayed injections - working around circular reference */
    protected abstract ServiceConfigurator getServiceConfigurator();
    protected abstract ReplicationManager getReplicationManager();
    protected abstract DomainConfiguration getDomainConfiguration();
    protected abstract ConfigGenerator getDialPlanConfigGenerator();

    /** Return the replication URLs, retrieving them on demand */
    @Override
    public Location[] getLocations() {
        List<Location> locationList = getHibernateTemplate().loadAll(Location.class);
        Collections.sort(locationList, new Comparator<Location>() {

            @Override
            public int compare(Location o1, Location o2) {
                return o1.getId() - o2.getId();
            }
        });
        Location[] locationArray = new Location[locationList.size()];
        locationList.toArray(locationArray);
        return locationArray;
    }

    public List<Location> getLocationsList() {
        return getHibernateTemplate().loadAll(Location.class);
    }

    @Override
    public Location getLocation(int id) {
        return getHibernateTemplate().load(Location.class, id);
    }

    @Override
    public Location getLocationByFqdn(String fqdn) {
        return loadLocationByUniqueProperty(LOCATION_PROP_NAME, fqdn);
    }

    @Override
    public Location getLocationByAddress(String address) {
        return loadLocationByUniqueProperty("address", address);
    }

    private Location loadLocationByUniqueProperty(String propName, Object propValue) {
        final Criterion expression = Restrictions.eq(propName, propValue);

        HibernateCallback callback = new HibernateCallback() {
            @Override
            public Object doInHibernate(Session session) {
                Criteria criteria = session.createCriteria(Location.class).add(expression);
                return criteria.list();
            }
        };
        List<Location> locations = getHibernateTemplate().executeFind(callback);
        Location location = singleResult(locations);

        return location;
    }

    /**
     * Stores location without publishing events. Used for migrating locations.
     */
    @Override
    public void storeMigratedLocation(Location location) {
        getHibernateTemplate().saveOrUpdate(location);
    }

    @Override
    public void saveNatLocation(Location location, NatLocation nat) {
        location.setNat(nat);
        getHibernateTemplate().saveOrUpdate(location);
        // There is a 1-1 relation between Nat and Location
        nat.setLocation(location);
        getNatTraversalManager().activateNatLocation(location);
    }

    @Override
    public void saveLocation(Location location) {
        boolean sendProfiles = false;
        if (location.isNew()) {
            if (isFqdnOrIpInUseExceptThis(location)) {
                throw new UserException(DUPLICATE_FQDN_OR_IP, location.getFqdn(), location.getAddress());
            }
            location.fqdnOrIpHasChangedOnSave();
            location.setCallTraffic(true);
            location.setReplicateConfig(true);
            getHibernateTemplate().save(location);
        } else {
            boolean isFqdnOrIpChanged = isFqdnOrIpChanged(location);
            if (isFqdnOrIpChanged && isFqdnOrIpInUseExceptThis(location)) {
                throw new UserException(DUPLICATE_FQDN_OR_IP, location.getFqdn(), location.getAddress());
            }
            if (location.isCallTraffic() && !location.isReplicateConfig()) {
                throw new UserException("&error.replication.config", location.getFqdn(), location.getAddress());
            }
            if (location.isPrimary() && !location.isReplicateConfig()) {
                throw new UserException("&error.primary.config", location.getFqdn(), location.getAddress());
            }
            location.fqdnOrIpHasChangedOnSave();
            if (getOriginalValue(location, "replicateConfig").equals(Boolean.FALSE) && location.isReplicateConfig()) {
                sendProfiles = true;
            }
            getHibernateTemplate().update(location);
            if (sendProfiles) {
                getServiceConfigurator().sendProfiles(Collections.singletonList(location));
            }
        }
    }

    /**
     * Use thie method when you want to update location ant not to be intercepted by DAO events
     * @param location
     */
    @Override
    public void updateLocation(Location location) {
        getHibernateTemplate().merge(location);
    }

    /**
     * Need to verify if existing fqdn or ip are about to be changed in order to be in sync with
     * potential situations for versions before 4.1.6 when an user may have at least two locations
     * with the same fqdn or ip. (This situation probably will never appear but we have to be
     * sure). If no ip/fqdn change occurs, no user exception is thrown no matter if there is at
     * least one more location with the same ip or fqdn
     */
    public boolean isFqdnOrIpChanged(Location location) {
        List count = getHibernateTemplate().findByNamedQueryAndNamedParam("sameLocationWithSameFqdnOrIp",
                new String[] {
                    LOCATION_PROP_ID, LOCATION_PROP_NAME, LOCATION_PROP_IP
                }, new Object[] {
                    location.getId(), location.getFqdn(), location.getAddress()
                });

        return intResult(count) == 0;
    }

    private boolean isFqdnOrIpInUseExceptThis(Location location) {
        List count = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "anotherLocationWithSameFqdnOrIpExceptThis", new String[] {
                    LOCATION_PROP_ID, LOCATION_PROP_NAME, LOCATION_PROP_IP
                }, new Object[] {
                    location.getId(), location.getFqdn(), location.getAddress()
                });

        return intResult(count) > 0;
    }

    @Override
    public void deleteLocation(Location location) {
        if (location.isPrimary()) {
            throw new UserException("&error.delete.primary", location.getFqdn());
        }
        getHibernateTemplate().delete(location);
        getHibernateTemplate().flush();
    }

    @Override
    public Location getPrimaryLocation() {
        return loadLocationByUniqueProperty(LOCATION_PROP_PRIMARY, true);
    }

    public void setDaoEventPublisher(DaoEventPublisher daoEventPublisher) {
        m_daoEventPublisher = daoEventPublisher;
    }
}
