/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Set;
import java.util.TreeSet;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.Location.State;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.logging.AuditLogContext;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.ReplicationsFinishedEvent;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdExtension;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class ReplicationTrigger extends SipxHibernateDaoSupport implements ApplicationListener, DaoEventListener {
    protected static final Log LOG = LogFactory.getLog(ReplicationTrigger.class);

    private CoreContext m_coreContext;
    private ReplicationManager m_replicationManager;
    private OpenAcdContext m_openAcdContext;
    private LocationsManager m_locationsManager;
    private AuditLogContext m_auditLogContext;
    private SipxReplicationContext m_lazySipxReplicationContext;

    /** no replication at start-up by default */
    private boolean m_replicateOnStartup;

    public boolean isReplicateOnStartup() {
        return m_replicateOnStartup;
    }

    public void setReplicateOnStartup(boolean replicateOnStartup) {
        m_replicateOnStartup = replicateOnStartup;
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setAuditLogContext(AuditLogContext auditLogContext) {
        m_auditLogContext = auditLogContext;
    }

    @Override
    public void onSave(Object entity) {
        if (entity instanceof Replicable) {
            m_replicationManager.replicateEntity((Replicable) entity);
            if (entity instanceof OpenAcdExtension) {
                getHibernateTemplate().flush();
                m_openAcdContext.replicateConfig();
            }
        } else if (entity instanceof Group) {
            generateGroup((Group) entity);
        } else if (entity instanceof Branch) {
            generateBranch((Branch) entity);
        } else if (entity instanceof Location) {
            m_replicationManager.replicateLocation((Location) entity);
        } else if (entity instanceof Permission) {
            generatePermission((Permission) entity);
        } else if (entity instanceof SpeedDial) {
            User u = ((SpeedDial) entity).getUser();
            m_replicationManager.replicateEntity(u, DataSet.SPEED_DIAL);
        }
    }

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof Replicable) {
            m_replicationManager.removeEntity((Replicable) entity);
        } else if (entity instanceof Group) {
            generateGroup((Group) entity);
        } else if (entity instanceof Branch) {
            generateBranch((Branch) entity);
        } else if (entity instanceof Location) {
            m_replicationManager.removeLocation((Location) entity);
        } else if (entity instanceof ArrayList< ? >) {
            ArrayList< ? > col = (ArrayList< ? >) entity;
            if (col.get(0) instanceof Gateway) {
                for (Object object : col) {
                    Gateway gw = (Gateway) object;
                    m_replicationManager.removeEntity(gw);
                }
            }
        } else if (entity instanceof Permission) {
            removePermission((Permission) entity);
        }
    }

    private void generateGroup(Group group) {
        if ("user".equals(group.getResource())) {
            for (User user : m_coreContext.getGroupMembers(group)) {
                m_replicationManager.replicateEntity(user);
            }
        }
    }

    private void generateBranch(Branch branch) {
        for (User user : m_coreContext.getUsersForBranch(branch)) {
            m_replicationManager.replicateEntity(user);
        }
    }

    private void generatePermission(Permission permission) {
        Object originalDefaultValue = getOriginalValue(permission, "defaultValue");
        if (originalDefaultValue == null) {
            if (!permission.getDefaultValue()) {
                return;
            } else {
                // We do not need lazy/async here. The operation uses mongo commands and does not
                // hit PG db.
                // It will take a matter of seconds and the control is taken safely to the page.
                // (i.e. we do not need to worry about timeout.)
                m_replicationManager.addPermission(permission);
                return;
            }
        }

        if ((Boolean) originalDefaultValue == permission.getDefaultValue()) {
            return;
        }
        m_lazySipxReplicationContext.generateAll(DataSet.PERMISSION);
    }

    private void removePermission(Permission permission) {
        // We do not need lazy/async here. The operation uses mongo commands and does not hit PG
        // db.
        // It will take a matter of seconds and the control is taken safely to the page.
        // (i.e. we do not need to worry about timeout.)
        m_replicationManager.removePermission(permission);
    }

    /**
     * Override ApplicationListener.onApplicationEvent so we can handle events.
     */
    @Override
    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof ApplicationInitializedEvent && isReplicateOnStartup()) {
            LOG.info("Replicating all data sets after application has initialized");
            m_replicationManager.replicateAllData();
        } else if (event instanceof ReplicationsFinishedEvent) {
            updateLocations();
        }
    }

    private void updateLocations() {
        Location[] locations = m_locationsManager.getLocations();
        for (Location location : locations) {
            // location is updated when SendProfiles finished execution and also
            // if/when any files get replicated in other scenarios based on
            // AuditLogContext worker's reports
            // such as when firstRun task is executed or occasional replications
            // take place when system is up and running
            Set<String> failedList = m_auditLogContext.getReplicationFailedList(location.getFqdn());
            Set<String> successfulList = m_auditLogContext.getReplicationSuccededList(location.getFqdn());
            // no replication occured for this location
            if ((failedList == null || failedList.isEmpty()) && (successfulList == null || successfulList.isEmpty())) {
                continue;
            } else if (failedList != null && !failedList.isEmpty()) {
                // when something failed, we have configuration error - we will
                // intersect current failures/successes with latest
                // with this scenario, sendProfiles may never get finished
                Set<String> prevSuccessfulList = location.getSuccessfulReplications();
                Set<String> prevFailedList = location.getFailedReplications();
                if (successfulList != null && !successfulList.isEmpty()) {
                    prevSuccessfulList.addAll(successfulList);
                    prevFailedList.removeAll(successfulList);
                }
                if (failedList != null && !failedList.isEmpty()) {
                    prevSuccessfulList.removeAll(failedList);
                    prevFailedList.addAll(failedList);
                }
                location.setLastAttempt(new Timestamp(Calendar.getInstance().getTimeInMillis()));
                location.setState(State.CONFIGURATION_ERROR);
                location.setSuccessfulReplications(prevSuccessfulList);
                location.setFailedReplications(prevFailedList);
                // location is configured only when sendProfiles successfully
                // finished and nothing failed
            } else if (!m_auditLogContext.isSendProfilesInProgress(location)) {
                location.setLastAttempt(new Timestamp(Calendar.getInstance().getTimeInMillis()));
                location.setState(State.CONFIGURED);
                location.setSuccessfulReplications(successfulList);
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

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setReplicationManager(ReplicationManager replicationManager) {
        m_replicationManager = replicationManager;
    }

    public void setOpenAcdContext(OpenAcdContext openAcdContext) {
        m_openAcdContext = openAcdContext;
    }

    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_lazySipxReplicationContext = sipxReplicationContext;
    }
}
