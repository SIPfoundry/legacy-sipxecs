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

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.sql.Timestamp;
import java.util.Calendar;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.Criteria;
import org.hibernate.Session;
import org.hibernate.criterion.Criterion;
import org.hibernate.criterion.Restrictions;
import org.sipfoundry.sipxconfig.common.ReplicationsFinishedEvent;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location.State;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.logging.AuditLogContext;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.orm.hibernate3.HibernateCallback;

public class LocationsManagerImpl extends SipxHibernateDaoSupport<Location> implements LocationsManager,
        ApplicationListener<ApplicationEvent>, SetupListener {
    public static final Log LOG = LogFactory.getLog(LocationsManagerImpl.class);
    private static final String LOCATION_PROP_NAME = "fqdn";
    private static final String LOCATION_PROP_PRIMARY = "primary";
    private static final String LOCATION_PROP_IP = "ipAddress";
    private static final String LOCATION_PROP_ID = "locationId";
    private static final String DUPLICATE_FQDN_OR_IP = "&error.duplicateFqdnOrIp";
    private AuditLogContext m_auditLogContext;
    private DomainManager m_domainManager;
    private String m_defaultStunServer = "stun.ezuce.com";

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
     *
     * @Override public void storeMigratedLocation(Location location) {
     *           getHibernateTemplate().saveOrUpdate(location); }
     */

    @Override
    public void saveLocation(Location location) {
        boolean sendProfiles = false;
        if (location.isNew()) {
            if (isFqdnOrIpInUseExceptThis(location)) {
                throw new UserException(DUPLICATE_FQDN_OR_IP, location.getFqdn(), location.getAddress());
            }
            location.fqdnOrIpHasChangedOnSave();
            location.setCallTraffic(true);
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
        // ARGH!! Kept getting duplicate object in session and usual tricks of merge and evict
        // didn't work so i resorted to SQL. --Douglas
        Location merge = getHibernateTemplate().merge(location);
        getHibernateTemplate().delete(merge);
//        // ARGH!! Kept getting duplicate object in session and usual tricks of merge and evict
//        // didn't work so i resorted to SQL. --Douglas
//        m_jdbcTemplate.execute("delete from location where location_id = " + location.getId());
//        getHibernateTemplate().flush();
    }

    @Override
    public Location getPrimaryLocation() {
        return loadLocationByUniqueProperty(LOCATION_PROP_PRIMARY, true);
    }

    /**
     * Override ApplicationListener.onApplicationEvent so we can handle events.
     */
    @Override
    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof ReplicationsFinishedEvent) {
            updateLocations();
        }
    }

    private void updateLocations() {
        Location[] locations = getLocations();
        for (Location location : locations) {
            // location is updated when SendProfiles finished execution and also
            // if/when any files get replicated in other scenarios based on
            // AuditLogContext worker's reports
            // such as when firstRun task is executed or occasional replications
            // take place when system is up and running
            Set<String> failedList = m_auditLogContext.getReplicationFailedList(location.getFqdn());
            if (failedList != null && !failedList.isEmpty()) {
                // when something failed, we have configuration error
                Set<String> prevFailedList = location.getFailedReplications();
                prevFailedList.addAll(failedList);
                location.setLastAttempt(new Timestamp(Calendar.getInstance().getTimeInMillis()));
                location.setState(State.CONFIGURATION_ERROR);
                location.setFailedReplications(prevFailedList);
                // location is configured only when sendProfiles successfully
                // finished and nothing failed
            } else if (!m_auditLogContext.isSendProfilesInProgress(location)) {
                location.setLastAttempt(new Timestamp(Calendar.getInstance().getTimeInMillis()));
                location.setState(State.CONFIGURED);
                location.setFailedReplications(new TreeSet<String>());
            } else {
                // in this case means that nothing failed at this point and we are
                // in send profiles progress...
                // we don't have to do anything because we have to wait until send
                // profiles finishes
                continue;
            }
            getHibernateTemplate().update(location);
        }
    }

    @Override
    public void setup(SetupManager manager) {
        String id = "init-locations";
        if (manager.isTrue(id)) {
            return;
        }
        Location[] locations = getLocations();
        if (locations.length > 0) {
            manager.setTrue(id);
            return;
        }

        try {
            String fqdn = InetAddress.getLocalHost().getHostName();
            String ip = InetAddress.getLocalHost().getHostAddress();
            Location primary = new Location(fqdn, ip);
            primary.setPrimary(true);
            primary.setName("Primary");
            primary.setStunAddress(m_defaultStunServer);
            saveLocation(primary);

            manager.setTrue(id);
        } catch (UnknownHostException e) {
            throw new RuntimeException("Could not determine host name and/or ip address, check /etc/hosts file", e);
        }
    }

    @Required
    public void setAuditLogContext(AuditLogContext auditLogContext) {
        m_auditLogContext = auditLogContext;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public void setDefaultStunServer(String defaultStunServer) {
        m_defaultStunServer = defaultStunServer;
    }
}
