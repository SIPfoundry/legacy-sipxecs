/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.commserver;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.common.LazyDaemon;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.springframework.context.ApplicationEvent;

/**
 * This context deals with lazy replication of configuration files. Configuration files need to be
 * lazily replicated, however entities do not Until mongo introduction it made sense for both IMDB
 * and config files to be lazily replicated but now with IMDB change to mongo and with the focus
 * shift from Dataset replication to Replicable entity replication lazy does not make sense for
 * entities. TODO: this will have to go away, and only eager replication to exist
 */
public class LazySipxReplicationContextImpl implements SipxReplicationContext {
    private static final Log LOG = LogFactory.getLog(LazySipxReplicationContextImpl.class);
    /**
     * 7s is the default sleep interval after any replication request is issued
     */
    private static final int DEFAULT_SLEEP_INTERVAL = 7000;

    private final Set<ReplicationTask> m_tasks = new HashSet<ReplicationTask>();

    private final List m_events = new ArrayList();

    private SipxReplicationContext m_target;

    private int m_sleepInterval = DEFAULT_SLEEP_INTERVAL;

    /**
     * Do not initialized worker here - properties must be set first
     */
    private Worker m_worker;

    public void init() {
        m_worker = new Worker();
        m_worker.start();
    }

    @Override
    public void generateAll() {
        m_target.generateAll();
    }

    @Override
    public synchronized void generateAll(DataSet ds) {
        m_tasks.add(new GenerateDatasetTask(ds));
        notifyWorker();
    }

    @Override
    public void generate(Replicable entity) {
        m_target.generate(entity);
    }

    @Override
    public void remove(Replicable entity) {
        m_target.remove(entity);
    }

    @Override
    public synchronized void publishEvent(ApplicationEvent event) {
        m_events.add(event);
        // we call notify and not notifyWorker - publishing event is not real work
        LOG.debug("Notify work on published event");
        notify();
    }

    private void notifyWorker() {
        m_worker.workScheduled();
        LOG.debug("Notify work scheduled");
        notify();
    }

    private synchronized void waitForWork() throws InterruptedException {
        if (m_tasks.isEmpty()) {
            LOG.debug("Waiting for work");
            wait();
            LOG.debug("Work received");
        }
    }

    private void publishQueuedEvents() {
        for (Iterator u = m_events.iterator(); u.hasNext();) {
            ApplicationEvent event = (ApplicationEvent) u.next();
            m_target.publishEvent(event);
        }
        m_events.clear();
    }

    private synchronized Set<ReplicationTask> getTasks() {
        if (m_tasks.isEmpty()) {
            return Collections.emptySet();
        }
        Set<ReplicationTask> tasks = new HashSet<ReplicationTask>(m_tasks.size());
        for (ReplicationTask task : m_tasks) {
            addOrUpdateTask(tasks, task);
        }
        m_tasks.clear();
        return tasks;
    }

    private void addOrUpdateTask(Set<ReplicationTask> tasks, ReplicationTask task) {
        for (ReplicationTask t : tasks) {
            if (t.update(task)) {
                // no need to add anything - existing task updated
                return;
            }
        }
        tasks.add(task);
    }

    public void setTarget(SipxReplicationContext target) {
        m_target = target;
    }

    public void setSleepInterval(int sleepInterval) {
        m_sleepInterval = sleepInterval;
    }

    /**
     * Worker thread: waits till someone needs to be done, sleeps for a bit and does it.
     */
    private class Worker extends LazyDaemon {
        public Worker() {
            super("Replication worker thread", m_sleepInterval);
        }

        @Override
        protected void waitForWork() throws InterruptedException {
            LazySipxReplicationContextImpl.this.waitForWork();
        }

        @Override
        protected boolean work() {
            for (ReplicationTask task : getTasks()) {
                task.replicate(m_target);
            }
            // before we start waiting publish all the events that are in the queue
            publishQueuedEvents();
            return true;
        }
    }

    abstract static class ReplicationTask {
        public abstract void replicate(SipxReplicationContext replicationContext);

        public abstract boolean update(ReplicationTask task);
    }

    static class GenerateDatasetTask extends ReplicationTask {
        private DataSet m_ds;

        public GenerateDatasetTask(DataSet ds) {
            m_ds = ds;
        }

        @Override
        public void replicate(SipxReplicationContext replicationContext) {
            replicationContext.generateAll(m_ds);

        }

        @Override
        public boolean update(ReplicationTask task) {
            return false;
        }

    }

    @Override
    public void replicateWork(Replicable entity) {
    }

}
