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
import java.util.Collection;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.ReplicationManager;
import org.sipfoundry.sipxconfig.admin.forwarding.CallSequence;
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
    private ApplicationEventPublisher m_applicationEventPublisher;
    private ReplicationManager m_replicationManager;
    private JobContext m_jobContext;
    private LocationsManager m_locationsManager;
    private ExecutorService m_fileExecutorService = Executors.newFixedThreadPool(5);

    protected abstract ServiceConfigurator getServiceConfigurator();

    @Override
    public void generate(final Replicable entity) {
        m_replicationManager.replicateEntity(entity);
    }

    @Override
    public void generateAll() {
        ReplicateWork work = new ReplicateWork() {
            @Override
            public boolean replicate() {
                return m_replicationManager.replicateAllData();
            }

        };
        doWithJob(IMDB_REGENERATION, m_locationsManager.getPrimaryLocation(), work);
    }

    @Override
    public void generateAll(DataSet ds) {
        m_replicationManager.replicateAllData(ds);
    }

    @Override
    public void replicateLocation(final Location location) {
        ReplicateWork work = new ReplicateWork() {
            @Override
            public boolean replicate() {
                return m_replicationManager.replicateLocation(location);
            }
        };
        doWithJob(SipxReplicationContext.MONGO_LOCATION_REGISTRATION, m_locationsManager.getPrimaryLocation(), work);
    }

    @Override
    public void remove(final Replicable entity) {
        m_replicationManager.removeEntity(entity);
    }

    @Override
    public void resyncSlave(Location location) {
        m_replicationManager.resyncSlave(location);
    }

    @Override
    public void replicate(ConfigurationFile file) {
        if (inInitializationPhase()) {
            LOG.debug(IGNORE_REPLICATION_MESSAGE + file.getName());
            return;
        }

        Location[] locations = m_locationsManager.getLocations();
        replicate(locations, file);
    }

    @Override
    public void replicate(Location[] locations, ConfigurationFile file) {
        for (Location location : locations) {
            replicate(location, file);
        }
    }

    @Override
    public void replicate(Location location, ConfigurationFile file) {
        if (inInitializationPhase()) {
            LOG.debug(IGNORE_REPLICATION_MESSAGE + file.getName());
            return;
        }

        replicateWorker(location, file);
    }

    private void replicateWorker(final Location location, final ConfigurationFile file) {
        ReplicateWork work = new ReplicateWork() {
            @Override
            public boolean replicate() {
                return m_replicationManager.replicateFile(location, file);
            }

        };
        doWithJobMultiThread("File replication: " + file.getName(), location, work);
    }

    public void regenerateCallSequences(final Collection<CallSequence> callSequences) {
        ReplicateWork work = new ReplicateWork() {
            @Override
            public boolean replicate() {
                try {
                    for (CallSequence callSequence : callSequences) {
                        m_replicationManager.replicateEntity(callSequence);
                    }
                } catch (Throwable t) {
                    return false;
                }
                return true;
            }
        };
        doWithJob("DST change: regeneration of call sequences.",
                m_locationsManager.getPrimaryLocation(), work);
    }

    private void doWithJobMultiThread(final String jobName, final Location location, final ReplicateWork work) {
        m_fileExecutorService.execute(new Runnable() {
            @Override
            public void run() {
                doWithJob(jobName, location, work);
            }
        });
    }

    private void doWithJob(final String jobName, final Location location, final ReplicateWork work) {
        Serializable jobId = m_jobContext.schedule(jobName, location);
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

    @Override
    @Required
    public void setApplicationEventPublisher(ApplicationEventPublisher applicationEventPublisher) {
        m_applicationEventPublisher = applicationEventPublisher;
    }

    @Override
    public void publishEvent(ApplicationEvent event) {
        m_applicationEventPublisher.publishEvent(event);
    }

}
