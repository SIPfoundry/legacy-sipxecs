/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.LazyDaemon;
import org.springframework.beans.factory.annotation.Required;

public class LazyDialPlanActivationManager implements DialPlanActivationManager {
    private static final Log LOG = LogFactory.getLog(LazyDialPlanActivationManager.class);

    /**
     * 15s is the default sleep interval after any replication request is issued
     */
    private static final int DEFAULT_SLEEP_INTERVAL = 15000;

    private int m_sleepInterval = DEFAULT_SLEEP_INTERVAL;

    private boolean m_replicate;

    private boolean m_restart;

    private DialPlanActivationManager m_target;

    private Worker m_worker;

    public synchronized void replicateDialPlan(boolean restartSbcDevices) {
        m_replicate = true;
        m_restart = m_restart || restartSbcDevices;
        notifyWorker();
    }

    public void init() {
        m_worker = new Worker();
        m_worker.start();
    }

    private void notifyWorker() {
        m_worker.workScheduled();
        LOG.debug("Notify work scheduled");
        notify();
    }

    private synchronized void waitForWork() throws InterruptedException {
        if (!m_replicate) {
            LOG.debug("Waiting for work");
            wait();
            LOG.debug("Work received");
        }
    }

    private synchronized boolean getRestart() {
        boolean restart = m_restart;
        m_replicate = false;
        m_restart = false;
        return restart;
    }

    /**
     * Worker thread: waits till someone needs to be done, sleeps for a bit and does it.
     */
    private class Worker extends LazyDaemon {
        public Worker() {
            super("Dial plan activation worker thread", m_sleepInterval);
        }

        @Override
        protected void waitForWork() throws InterruptedException {
            LazyDialPlanActivationManager.this.waitForWork();
        }

        @Override
        protected boolean work() {
            boolean restart = getRestart();
            m_target.replicateDialPlan(restart);
            return true;
        }
    }

    @Required
    public void setTarget(DialPlanActivationManager target) {
        m_target = target;
    }

    public void setSleepInterval(int sleepInterval) {
        m_sleepInterval = sleepInterval;
    }
}
