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

import java.util.ArrayList;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdExtension;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class ReplicationTrigger implements ApplicationListener, DaoEventListener {
    protected static final Log LOG = LogFactory.getLog(ReplicationTrigger.class);

    private CoreContext m_coreContext;
    private ReplicationManager m_replicationManager;
    private OpenAcdContext m_openAcdContext;

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
            if (entity instanceof OpenAcdExtension) {
                m_openAcdContext.replicateConfig();
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

    public void setReplicationManager(ReplicationManager replicationManager) {
        m_replicationManager = replicationManager;
    }

    public void setOpenAcdContext(OpenAcdContext openAcdContext) {
        m_openAcdContext = openAcdContext;
    }
}
