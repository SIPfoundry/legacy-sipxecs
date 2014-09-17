/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.commserver.imdb;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.rls.Rls;
import org.sipfoundry.sipxconfig.setting.Group;

/**
 * This listener is responsible for triggering mongo replication of some entities. Its most
 * important job is to trigger replication of Replicable entities. It also takes care of the
 * replication of groups of Replicable entities such as Group or Branch.
 */
// PLEASE keep this class clean. Keep in mind this is not a multi-purpose listener and do not
// over-use.
// If specific packages need to do specific things based on save/delete to specific objects, that
// logic
// should exist with the project.
public class ReplicationTrigger extends SipxHibernateDaoSupport implements DaoEventListener {
    protected static final Log LOG = LogFactory.getLog(ReplicationTrigger.class);

    private ReplicationManager m_replicationManager;
    private ExecutorService m_executorService;
    private ConfigManager m_configManager;
    private FeatureManager m_featureManager;

    @Override
    public void onSave(Object entity) {
        if (entity instanceof Replicable) {
            if (entity instanceof Group) {
                // flush is necessary here in order to get consistent data
                getHibernateTemplate().flush();
                // It is important to replicate asynch since large groups might take a while to
                // replicate
                // and we want to return control to the page immediately.
                replicateEntityGroup(new GroupWorker(entity));
            }
            m_replicationManager.replicateEntity((Replicable) entity);
        } else if (entity instanceof Branch) {
            getHibernateTemplate().flush();
            // there is no file replication needed so we can trigger the branch replication
            // directly
            replicateEntityGroup(new BranchWorker(entity));
        }
    }

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof Replicable) {
            if (entity instanceof Group) {
                // It is important to replicate asynch since large groups might take a while to
                // replicate
                // and we want to return control to the page immadiately.
                replicateEntityGroup(new GroupDeleteWorker(entity));
            }
            m_replicationManager.removeEntity((Replicable) entity);
        } else if (entity instanceof Branch) {
            replicateEntityGroup(new BranchDeleteWorker(entity));
        }
    }

    /*
     * Runnables that call the actual replication to be submited to the ExecutorService
     */
    private class BranchWorker implements Runnable {
        private final Object m_entity;

        public BranchWorker(Object entity) {
            m_entity = entity;
        }

        @Override
        public void run() {
            m_replicationManager.replicateBranch((Branch) m_entity);
        }
    }

    // public for use in tests
    public class BranchDeleteWorker implements Runnable {
        private final Object m_entity;

        public BranchDeleteWorker(Object entity) {
            m_entity = entity;
        }

        @Override
        public void run() {
            m_replicationManager.deleteBranch((Branch) m_entity);
        }
    }

    private class GroupWorker implements Runnable {
        private final Object m_entity;

        public GroupWorker(Object entity) {
            m_entity = entity;
        }

        @Override
        public void run() {
            generateGroup((Group) m_entity);
        }
    }

    private class GroupDeleteWorker implements Runnable {
        private final Object m_entity;

        public GroupDeleteWorker(Object entity) {
            m_entity = entity;
        }

        @Override
        public void run() {
            deleteGroup((Group) m_entity);
        }
    }

    // ensure async replication of groups of entities
    // can be used for groups, as well as branches (don't let the name fool you)
    private void replicateEntityGroup(Runnable worker) {
        if (m_executorService == null) {
            m_executorService = Executors.newSingleThreadExecutor(Executors.defaultThreadFactory());
        }
        m_executorService.submit(worker);
        m_executorService.shutdown();
        m_executorService = null;
    }

    /**
     * Sequence of replication actions that need to be performed when a group is saved. Order of
     * the sequence is important - files must be replicated after group members.
     *
     * @param group
     */
    private void generateGroup(Group group) {
        if (User.GROUP_RESOURCE_ID.equals(group.getResource())) {
            m_replicationManager.replicateGroup(group);
            activateGroup();
        }
        if (Phone.GROUP_RESOURCE_ID.equals(group.getResource())) {
            m_replicationManager.replicatePhoneGroup(group);
        }
    }

    /**
     * Sequence of replication actions that need to be performed when a group is deleted. Order of
     * the sequence is important - files must be replicated after group members.
     *
     * @param group
     */
    private void deleteGroup(Group group) {
        m_replicationManager.deleteGroup(group);
        if (User.GROUP_RESOURCE_ID.equals(group.getResource())) {
            activateGroup();
        }
    }

    /**
     * Helper method to replicate files when group is saved/removed.
     */
    private void activateGroup() {
        if (m_featureManager.isFeatureEnabled(ImManager.FEATURE)) {
            m_configManager.configureEverywhere(ImManager.FEATURE);
        }
        if (m_featureManager.isFeatureEnabled(Rls.FEATURE)) {
            m_configManager.configureEverywhere(Rls.FEATURE);
        }
    }

    public void setReplicationManager(ReplicationManager replicationManager) {
        m_replicationManager = replicationManager;
    }

    /**
     * use only in tests
     *
     * @param executorService
     */
    public void setExecutorService(ExecutorService executorService) {
        m_executorService = executorService;
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }
}
