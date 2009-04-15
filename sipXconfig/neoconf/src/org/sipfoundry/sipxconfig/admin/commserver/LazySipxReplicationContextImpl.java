/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.common.LazyDaemon;
import org.sipfoundry.sipxconfig.common.ReplicationsFinishedEvent;
import org.springframework.context.ApplicationEvent;

public class LazySipxReplicationContextImpl implements SipxReplicationContext {
    private static final Log LOG = LogFactory.getLog(LazySipxReplicationContextImpl.class);
    /**
     * 7s is the default sleep interval after any replication request is issued
     */
    private static final int DEFAULT_SLEEP_INTERVAL = 7000;

    private final List<ReplicationTask> m_tasks = new ArrayList<ReplicationTask>();

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

    public synchronized void generate(DataSet dataSet) {
        m_tasks.add(new DataSetTask(dataSet));
        notifyWorker();
    }

    public synchronized void generateAll() {
        List<DataSet> dataSets = DataSet.getEnumList();
        for (DataSet dataSet : dataSets) {
            m_tasks.add(new DataSetTask(dataSet));
        }
        notifyWorker();
    }

    public synchronized void replicate(ConfigurationFile conf) {
        m_tasks.add(new ConfTask(conf));
        notifyWorker();
    }

    public void replicate(Location location, ConfigurationFile conf) {
        m_tasks.add(new ConfTask(location, conf));
        notifyWorker();
    }

    public String getXml(DataSet dataSet) {
        return m_target.getXml(dataSet);
    }

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
        m_target.publishEvent(new ReplicationsFinishedEvent(this));
        m_events.clear();
    }

    private synchronized List<ReplicationTask> getTasks() {
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

    static class DataSetTask extends ReplicationTask {
        private final DataSet m_ds;

        DataSetTask(DataSet ds) {
            m_ds = ds;
        }

        @Override
        public void replicate(SipxReplicationContext replicationContext) {
            replicationContext.generate(m_ds);
        }

        @Override
        public boolean update(ReplicationTask task) {
            if (task instanceof DataSetTask) {
                DataSetTask dst = (DataSetTask) task;
                return m_ds.equals(dst.m_ds);
            }
            return false;
        }
    }

    static class ConfTask extends ReplicationTask {
        /**
         * list of locations to replicate configuration on null means all locations here...
         */
        private List<Location> m_locations;
        private final ConfigurationFile m_conf;

        public ConfTask(ConfigurationFile conf) {
            m_conf = conf;
        }

        public ConfTask(Location location, ConfigurationFile conf) {
            m_conf = conf;
            m_locations = new ArrayList<Location>();
            m_locations.add(location);
        }

        @Override
        public void replicate(SipxReplicationContext replicationContext) {
            if (m_locations == null) {
                replicationContext.replicate(m_conf);
            } else {
                for (Location location : m_locations) {
                    replicationContext.replicate(location, m_conf);
                }
            }
        }

        @Override
        public boolean update(ReplicationTask task) {
            if (task instanceof ConfTask) {
                ConfTask ct = (ConfTask) task;
                if (m_conf.equals(ct.m_conf)) {
                    if (m_locations != null && ct.m_locations != null) {
                        m_locations.addAll(ct.m_locations);
                    } else {
                        m_locations = null;
                    }
                    return true;
                }
            }
            return false;
        }
    }
}
