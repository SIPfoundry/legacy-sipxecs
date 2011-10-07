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
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.Criteria;
import org.hibernate.Session;
import org.hibernate.criterion.Criterion;
import org.hibernate.criterion.Restrictions;
import org.sipfoundry.sipxconfig.acd.stats.AcdHistoricalConfigurationFile;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.ReplicationManager;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigGenerator;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.domain.DomainConfiguration;
import org.sipfoundry.sipxconfig.nattraversal.NatLocation;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversalManager;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxAlarmService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.SipxSupervisorService;
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
    /* delayed injections - working around circular reference */
    protected abstract ServiceConfigurator getServiceConfigurator();
    protected abstract ReplicationManager getReplicationManager();
    protected abstract SipxServiceManager getSipxServiceManager();
    protected abstract DomainConfiguration getDomainConfiguration();
    protected abstract AcdHistoricalConfigurationFile getAcdHistoricalConfiguration();
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
    public void saveServerRoleLocation(Location location, ServerRoleLocation role) {
        location.setServerRoles(role);
        getHibernateTemplate().saveOrUpdate(location);
        role.setLocation(location);
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
        getReplicationManager().registerTunnels(location);
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

    /**
     * Gets the set of items to be replicated on this location.
     * It is a function of the set of installed services,
     * dial plan rules and mongo replications.
     */
    public Set<String> getReplications(Location location) {
        Set<String> replications = new HashSet<String>();
        replications.addAll(Arrays.asList(location.getDefaultMongoReplications()));
        Collection<SipxService> services = location.getSipxServices();
        //Supervisor and alarm services are by default installed in every location
        services.add(getSipxServiceManager().getServiceByBeanId(SipxSupervisorService.BEAN_ID));
        services.add(getSipxServiceManager().getServiceByBeanId(SipxAlarmService.BEAN_ID));
        for (SipxService service : services) {
            Set<? extends ConfigurationFile> serviceConfigurations = service.getConfigurations();
            for (ConfigurationFile config : serviceConfigurations) {
                if (config.isReplicable(location)) {
                    replications.add(config.getName());
                }
            }
        }
        //Domain configuration is replicated by default. we can still check this.
        if (getDomainConfiguration().isReplicable(location)) {
            replications.add(getDomainConfiguration().getName());
        }
        //dial plans generation
        //there are a total of 4 rules files that will get generated and replicated
        //authrules.xml fallbackrules.xml forwardingrules.xml mappingrules.xml
        for (ConfigurationFile config : getDialPlanConfigGenerator().getRulesFiles()) {
            replications.add(config.getName());
        }

        if (getAcdHistoricalConfiguration().isReplicable(location)) {
            replications.add(getAcdHistoricalConfiguration().getName());
        }
        return replications;
    }
}
