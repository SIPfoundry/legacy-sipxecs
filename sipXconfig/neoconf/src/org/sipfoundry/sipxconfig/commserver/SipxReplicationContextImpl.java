/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.commserver;

import java.io.Serializable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.commserver.imdb.ReplicationManager;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.localization.LanguageUpdatedEvent;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationEventPublisher;
import org.springframework.context.ApplicationEventPublisherAware;
import org.springframework.context.ApplicationListener;

/*
 * (non-Javadoc)
 * @see org.sipfoundry.sipxconfig.commserver.SipxReplicationContext#generateAll()
 * all heavy replication operations need to be asynchronous, in order for the ontrol to be returned to the
 * calling page immediately. Otherwise a timeout will be, and an error page will be shown.
 * By heavy replication operations we mean operations that take a lot of time when are performed, usually
 * on a large number of users. (generate all data, generate 1 dataset)
 */
public class SipxReplicationContextImpl implements ApplicationEventPublisherAware, SipxReplicationContext,
        ApplicationListener<LanguageUpdatedEvent> {
    private static final Log LOG = LogFactory.getLog(SipxReplicationContextImpl.class);
    private ApplicationEventPublisher m_applicationEventPublisher;
    private ReplicationManager m_replicationManager;
    private JobContext m_jobContext;
    private LocationsManager m_locationsManager;

    @Override
    public void generate(final Replicable entity) {
        m_replicationManager.replicateEntity(entity);
    }

    @Override
    public void generateAll() {
        ExecutorService service = Executors.newSingleThreadExecutor();
        service.submit(new ReplicationWorker());
        service.shutdown();
    }

    private class ReplicationWorker implements Runnable {
        @Override
        public void run() {
            ReplicateWork work = new ReplicateWork() {
                @Override
                public void replicate() {
                    m_replicationManager.replicateAllData();
                }

            };
            doWithJob(IMDB_REGENERATION, m_locationsManager.getPrimaryLocation(), work);
        }
    }

    private class DatasetReplicationWorker implements Runnable {
        private final DataSet m_ds;

        public DatasetReplicationWorker(DataSet ds) {
            m_ds = ds;
        }

        @Override
        public void run() {
            m_replicationManager.replicateAllData(m_ds);
        }
    }

    @Override
    public void generateAll(final DataSet ds) {
        ExecutorService service = Executors.newSingleThreadExecutor();
        service.submit(new DatasetReplicationWorker(ds));
        service.shutdown();
    }

    @Override
    public void remove(final Replicable entity) {
        m_replicationManager.removeEntity(entity);
    }

    private void doWithJob(final String jobName, final Location location, final ReplicateWork work) {
        Serializable jobId = m_jobContext.schedule(jobName, location);
        try {
            LOG.info("Start replication: " + jobName);
            m_jobContext.start(jobId);
            work.replicate();
            m_jobContext.success(jobId);
        } catch (RuntimeException e) {
            LOG.warn("Replication failed: " + jobName, e);
            // there is not really a good info here - advise user to consult log?
            m_jobContext.failure(jobId, null, null);
        }
    }

    interface ReplicateWork {
        void replicate();
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

    @Override
    @Required
    public void setApplicationEventPublisher(ApplicationEventPublisher applicationEventPublisher) {
        m_applicationEventPublisher = applicationEventPublisher;
    }

    @Override
    public void publishEvent(ApplicationEvent event) {
        m_applicationEventPublisher.publishEvent(event);
    }

    @Override
    public void replicateWork(Replicable entity) {
        throw new RuntimeException("No clear what should happen here --Douglas");
    }

    @Override
    public void onApplicationEvent(LanguageUpdatedEvent arg0) {
        LOG.debug("Language updated, replicating DataSet.MAILSTORE...");
        generateAll(DataSet.MAILSTORE);
    }
}
