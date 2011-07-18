/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.Criteria;
import org.hibernate.Session;
import org.hibernate.criterion.Criterion;
import org.hibernate.criterion.Restrictions;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.nattraversal.NatLocation;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversalManager;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.springframework.orm.hibernate3.HibernateCallback;

import static org.springframework.dao.support.DataAccessUtils.intResult;
import static org.springframework.dao.support.DataAccessUtils.singleResult;

public abstract class LocationsManagerImpl extends SipxHibernateDaoSupport<Location> implements LocationsManager {
    public static final Log LOG = LogFactory.getLog(LocationsManagerImpl.class);
    private static final String LOCATION_PROP_NAME = "fqdn";
    private static final String LOCATION_PROP_PRIMARY = "primary";
    private static final String LOCATION_PROP_IP = "ipAddress";
    private static final String LOCATION_PROP_ID = "locationId";
    private static final String DUPLICATE_FQDN_OR_IP = "&error.duplicateFqdnOrIp";
    protected abstract NatTraversalManager getNatTraversalManager();

    /** Return the replication URLs, retrieving them on demand */
    @Override
    public Location[] getLocations() {
        List<Location> locationList = getHibernateTemplate().loadAll(Location.class);
        Location[] locationArray = new Location[locationList.size()];
        locationList.toArray(locationArray);
        return locationArray;
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
    public void saveServerRoleLocation(Location location, ServerRoleLocation role) {
        location.setServerRoles(role);
        getHibernateTemplate().saveOrUpdate(location);
        role.setLocation(location);
    }

    @Override
    public void saveLocation(Location location) {
        if (location.isNew()) {
            if (isFqdnOrIpInUseExceptThis(location)) {
                throw new UserException(DUPLICATE_FQDN_OR_IP, location.getFqdn(), location.getAddress());
            }
            location.fqdnOrIpHasChangedOnSave();
            getHibernateTemplate().save(location);
        } else {
            boolean isFqdnOrIpChanged = isFqdnOrIpChanged(location);
            if (isFqdnOrIpChanged && isFqdnOrIpInUseExceptThis(location)) {
                throw new UserException(DUPLICATE_FQDN_OR_IP, location.getFqdn(), location.getAddress());
            }
            location.fqdnOrIpHasChangedOnSave();
            getHibernateTemplate().update(location);
        }
    }

    /**
     * Use thie method when you want to update location ant not to be intercepted by DAO events
     * @param location
     */
    @Override
    public void updateLocation(Location location) {
        getHibernateTemplate().update(location);
        getHibernateTemplate().flush();
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
    }

    @Override
    public Location getPrimaryLocation() {
        return loadLocationByUniqueProperty(LOCATION_PROP_PRIMARY, true);
    }

    @Override
    public List<Location> getLocationsForService(SipxService service) {
        List<Location> locations = new ArrayList<Location>();
        for (Location location : getLocations()) {
            if (location.isServiceInstalled(service)) {
                locations.add(location);
            }
        }
        return locations;
    }

    @Override
    public Location getLocationByBundle(String bundleName) {
        List<Location> locations = getHibernateTemplate().findByNamedQueryAndNamedParam("locationsByBundle",
                "locationBundle", bundleName);
        if (locations.isEmpty()) {
            return null;
        }
        return locations.get(0);
    }
}
