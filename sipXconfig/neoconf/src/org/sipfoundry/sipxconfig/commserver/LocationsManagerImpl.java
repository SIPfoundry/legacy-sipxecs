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

import static java.lang.String.format;
import static org.springframework.dao.support.DataAccessUtils.intResult;
import static org.springframework.dao.support.DataAccessUtils.singleResult;

import java.sql.Timestamp;
import java.util.Calendar;
import java.util.Collections;
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
import org.springframework.jdbc.core.JdbcTemplate;
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
    private String m_defaultStunServer = "stun.ezuce.com";
    private String m_primaryFqdn;
    private String m_primaryIp;
    private JdbcTemplate m_jdbc;
    private DomainManager m_domainManager;

    /** Return the replication URLs, retrieving them on demand */
    @Override
    public Location[] getLocations() {
        List<Location> locationList = getHibernateTemplate().loadAll(Location.class);
        Collections.sort(locationList);
        Location[] locationArray = new Location[locationList.size()];
        locationList.toArray(locationArray);
        return locationArray;
    }

    @Override
    public List<Location> getLocationsList() {
        List<Location> locations = getHibernateTemplate().loadAll(Location.class);
        Collections.sort(locations);
        return locations;
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
        if (location.isNew()) {
            if (isFqdnOrIpInUseExceptThis(location)) {
                throw new UserException(DUPLICATE_FQDN_OR_IP, location.getFqdn(), location.getAddress());
            }
            location.setCallTraffic(true);
            getHibernateTemplate().save(location);
        } else {
            if (location.hasFqdnOrIpChangedOnSave() && isFqdnOrIpInUseExceptThis(location)) {
                throw new UserException(DUPLICATE_FQDN_OR_IP, location.getFqdn(), location.getAddress());
            }
            getHibernateTemplate().merge(location);
        }
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
        Location merge = getHibernateTemplate().merge(location);
        getHibernateTemplate().delete(merge);
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
            getHibernateTemplate().merge(location);
        }
    }

    @Override
    public boolean setup(SetupManager manager) {
        Location[] locations = getLocations();
        if (locations.length > 0) {
            for (Location l : locations) {
                if (l.isPrimary()) {
                    if (!l.getAddress().equals(m_primaryIp)) {
                        changePrimaryIp(m_primaryIp);
                    }
                    if (!l.getFqdn().equals(m_primaryFqdn)) {
                        changePrimaryFqdn(m_primaryFqdn);
                    }
                    break;
                }
            }
        } else {
            Location primary = new Location(m_primaryFqdn, m_primaryIp);
            primary.setPrimary(true);
            primary.setName("Primary");
            primary.setStunAddress(m_defaultStunServer);
            primary.setState(State.CONFIGURED);
            saveLocation(primary);
        }
        return true;
    }

    private void changePrimaryIp(String ip) {
        // Ran into problems exec-ing a stored proc, this gave error about
        // response when none was expected
        // m_jdbc.update(...
        //
        m_jdbc.execute(format("select change_primary_ip_on_restore('%s')", ip));
    }

    private void changePrimaryFqdn(String fqdn) {
        m_jdbc.execute(format("select change_primary_fqdn_on_restore('%s')", fqdn));
        // XX-10243 - clear domain aliases
        m_domainManager.setNullDomain();
    }

    @Required
    public void setAuditLogContext(AuditLogContext auditLogContext) {
        m_auditLogContext = auditLogContext;
    }

    public void setDefaultStunServer(String defaultStunServer) {
        m_defaultStunServer = defaultStunServer;
    }

    public void setPrimaryFqdn(String primaryFqdn) {
        m_primaryFqdn = primaryFqdn;
    }

    public void setPrimaryIp(String primaryIp) {
        m_primaryIp = primaryIp;
    }

    public void setJdbc(JdbcTemplate jdbc) {
        m_jdbc = jdbc;
    }

    public String getPrimaryFqdn() {
        return m_primaryFqdn;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }
}
