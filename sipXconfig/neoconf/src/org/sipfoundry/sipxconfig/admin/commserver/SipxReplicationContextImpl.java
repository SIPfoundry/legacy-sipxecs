/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.LazySipxReplicationContextImpl.DataSetTask;
import org.sipfoundry.sipxconfig.admin.commserver.LazySipxReplicationContextImpl.ReplicationTask;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.ReplicationManager;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationEventPublisher;
import org.springframework.context.ApplicationEventPublisherAware;

public abstract class SipxReplicationContextImpl implements ApplicationEventPublisherAware, SipxReplicationContext {

    private static final String IGNORE_REPLICATION_MESSAGE = "In initialization phase, ignoring request to replicate ";

    private static final Log LOG = LogFactory.getLog(SipxReplicationContextImpl.class);
    private final List<ReplicationTask> m_tasks = new ArrayList<ReplicationTask>();
    private ApplicationEventPublisher m_applicationEventPublisher;
    private ReplicationManager m_replicationManager;
    private JobContext m_jobContext;
    private LocationsManager m_locationsManager;

    protected abstract ServiceConfigurator getServiceConfigurator();

    public void generate(final Replicable entity) {
        /*
         * ReplicateWork work = new ReplicateWork() { public boolean replicate() { return
         * m_replicationManager.replicateEntity(entity); } }; doWithJob("Data replication: " +
         * entity.getName(), work);
         */
        m_tasks.add(new LazySipxReplicationContextImpl.DataSetTask(entity, false));
    }

    public void generateAll() {
        ReplicateWork work = new ReplicateWork() {
            public boolean replicate() {
                m_replicationManager.dropDb();
                return m_replicationManager.replicateAllData();
            }
        };
        doWithJob("Beggining data replication.", work);
    }

    public void remove(final Replicable entity) {
        /*
         * ReplicateWork work = new ReplicateWork() { public boolean replicate() { return
         * m_replicationManager.removeEntity(entity); } }; doWithJob("Replication - remove: " +
         * entity.getName(), work);
         */
        m_tasks.add(new LazySipxReplicationContextImpl.DataSetTask(entity, true));
    }

    public void replicate(ConfigurationFile file) {
        if (inInitializationPhase()) {
            LOG.debug(IGNORE_REPLICATION_MESSAGE + file.getName());
            return;
        }

        Location[] locations = m_locationsManager.getLocations();
        replicateWorker(locations, file);
    }

    public void replicate(Location location, ConfigurationFile file) {
        if (inInitializationPhase()) {
            LOG.debug(IGNORE_REPLICATION_MESSAGE + file.getName());
            return;
        }

        Location[] locations = new Location[] {
            location
        };
        replicateWorker(locations, file);
    }

    private void replicateWorker(final Location[] locations, final ConfigurationFile file) {
        ReplicateWork work = new ReplicateWork() {
            public boolean replicate() {
                return m_replicationManager.replicateFile(locations, file);
            }

        };
        doWithJob("File replication: " + file.getName(), work);
    }

    private void doWithJob(String jobName, ReplicateWork work) {
        Serializable jobId = m_jobContext.schedule(jobName);
        boolean success = false;
        try {
            LOG.info("Start replication: " + jobName);
            m_jobContext.start(jobId);
            success = work.replicate();
        } finally {
            if (success) {
                m_jobContext.success(jobId);
            } else {
                LOG.warn("Replication failed: " + jobName);
                // there is not really a good info here - advise user to consult log?
                m_jobContext.failure(jobId, null, null);
            }
        }
    }

    private boolean inInitializationPhase() {
        String initializationPhase = System.getProperty("sipxconfig.initializationPhase");
        if (initializationPhase == null) {
            return false;
        }

        return Boolean.parseBoolean(initializationPhase);
    }

    interface ReplicateWork {
        boolean replicate();
    }

    @Required
    public void setReplicationManager(ReplicationManager replicationManager) {
        m_replicationManager = replicationManager;
    }

    @Required
    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setApplicationEventPublisher(ApplicationEventPublisher applicationEventPublisher) {
        m_applicationEventPublisher = applicationEventPublisher;
    }

    public void publishEvent(ApplicationEvent event) {
        m_applicationEventPublisher.publishEvent(event);
    }

    private List<ReplicationTask> getTasks() {
        if (m_tasks.isEmpty()) {
            return Collections.emptyList();
        }
        List<ReplicationTask> tasks = new ArrayList<ReplicationTask>(m_tasks.size());
        for (ReplicationTask task : m_tasks) {
            addOrUpdateTask(tasks, task);
        }
        m_tasks.clear();
        return tasks;
    }

    private void addOrUpdateTask(List<ReplicationTask> tasks, ReplicationTask task) {
        for (ReplicationTask t : tasks) {
            if (t.update(task)) {
                // no need to add anything - existing task updated
                return;
            }
        }
        tasks.add(task);
    }

    public void replicateWork(final Replicable entity) {
        ReplicationTask taskToRemove = new DataSetTask();
        for (ReplicationTask task : m_tasks) {
            String taskid = ((DataSetTask) task).getEntity().getClass().getSimpleName()
                    + ((BeanWithId) ((DataSetTask) task).getEntity()).getId();
            String entityid = entity.getClass().getSimpleName() + ((BeanWithId) entity).getId();
            if (taskid.equals(entityid)) {
                if (((DataSetTask) task).isDelete()) {
                    m_replicationManager.removeEntity(entity);
                } else {
                    m_replicationManager.replicateEntity(entity);
                }

                taskToRemove = task;
            }
        }
        m_tasks.remove(taskToRemove);
    }

}
