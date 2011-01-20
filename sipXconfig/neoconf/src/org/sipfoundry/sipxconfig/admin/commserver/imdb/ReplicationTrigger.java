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
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.branch.BranchesWithUsersDeletedEvent;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class ReplicationTrigger implements ApplicationListener, DaoEventListener {
    protected static final Log LOG = LogFactory.getLog(ReplicationTrigger.class);

    private SipxReplicationContext m_replicationContext;
    private CoreContext m_coreContext;

    /** no replication at start-up by default */
    private boolean m_replicateOnStartup;

    @Required
    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    public boolean isReplicateOnStartup() {
        return m_replicateOnStartup;
    }

    public void setReplicateOnStartup(boolean replicateOnStartup) {
        m_replicateOnStartup = replicateOnStartup;
    }

    public void onSave(Object entity) {
        if (entity instanceof Replicable) {
            m_replicationContext.generate((Replicable) entity);
        } else if (entity instanceof Group) {
            generateGroup((Group) entity);
        } else if (entity instanceof Branch) {
            generateBranch((Branch) entity);
        }
    }

    public void onDelete(Object entity) {
        if (entity instanceof Replicable) {
            m_replicationContext.remove((Replicable) entity);
        } else if (entity instanceof Group) {
            generateGroup((Group) entity);

        } else if (entity instanceof Branch) {
            generateBranch((Branch) entity);
        }
    }

    private void generateGroup(Group group) {
        if ("user".equals(group.getResource())) {
            for (User user : m_coreContext.getGroupMembers(group)) {
                m_replicationContext.generate(user);
            }
        }
    }

    private void generateBranch(Branch branch) {
        for (User user : m_coreContext.getUsersForBranch(branch)) {
            m_replicationContext.generate(user);
        }
    }

    /**
     * Override ApplicationListener.onApplicationEvent so we can handle events. Perform data
     * replication every time the app starts up.
     */
    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof ApplicationInitializedEvent && isReplicateOnStartup()) {
            LOG.info("Replicating all data sets after application has initialized");
            m_replicationContext.generateAll();
        } else if (event instanceof BranchesWithUsersDeletedEvent) {
            // m_replicationContext.generate(); 0 find all users in branch and replicate them
            LOG.debug("aaaaaA");
        }
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

}
