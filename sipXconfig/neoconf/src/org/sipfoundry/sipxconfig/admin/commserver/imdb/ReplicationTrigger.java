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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.ValidUsersConfig;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class ReplicationTrigger implements ApplicationListener, DaoEventListener {
    protected static final Log LOG = LogFactory.getLog(ReplicationTrigger.class);

    private CoreContext m_coreContext;
    private SipxReplicationContext m_lazyReplicationContext;
    private ValidUsersConfig m_validUsersConfig;
    private ReplicationManager m_replicationManager;

    /** no replication at start-up by default */
    private boolean m_replicateOnStartup;

    public boolean isReplicateOnStartup() {
        return m_replicateOnStartup;
    }

    public void setReplicateOnStartup(boolean replicateOnStartup) {
        m_replicateOnStartup = replicateOnStartup;
    }

    public void onSave(Object entity) {
        if (entity instanceof Replicable) {
            m_replicationManager.replicateEntity((Replicable) entity);
            if (entity instanceof User) {
                m_lazyReplicationContext.replicate(m_validUsersConfig);
            }
        } else if (entity instanceof Group) {
            generateGroup((Group) entity);
        } else if (entity instanceof Branch) {
            generateBranch((Branch) entity);
        } else if (entity instanceof Location) {
            m_replicationManager.replicateLocation((Location) entity);
        }
    }

    public void onDelete(Object entity) {
        if (entity instanceof Replicable) {
            m_replicationManager.removeEntity((Replicable) entity);
            if (entity instanceof User) {
                m_lazyReplicationContext.replicate(m_validUsersConfig);
            }
        } else if (entity instanceof Group) {
            generateGroup((Group) entity);
        } else if (entity instanceof Branch) {
            generateBranch((Branch) entity);
        } else if (entity instanceof Location) {
            m_replicationManager.removeLocation((Location) entity);
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

    /**
     * Override ApplicationListener.onApplicationEvent so we can handle events. Perform data
     * replication every time the app starts up.
     */
    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof ApplicationInitializedEvent && isReplicateOnStartup()) {
            LOG.info("Replicating all data sets after application has initialized");
            m_replicationManager.replicateAllData();
        }
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setLazyReplicationContext(SipxReplicationContext lazySipxReplicationContext) {
        m_lazyReplicationContext = lazySipxReplicationContext;
    }

    public void setValidUsersConfig(ValidUsersConfig validUsersConfig) {
        m_validUsersConfig = validUsersConfig;
    }

    public void setReplicationManager(ReplicationManager replicationManager) {
        m_replicationManager = replicationManager;
    }
}
