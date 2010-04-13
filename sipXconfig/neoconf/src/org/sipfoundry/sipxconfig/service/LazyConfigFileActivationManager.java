/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.LazyDaemon;
import org.springframework.beans.factory.annotation.Required;

public abstract class LazyConfigFileActivationManager implements ConfigFileActivationManager {
    private static final Log LOG = LogFactory.getLog(LazyConfigFileActivationManager.class);

    /**
     * 10s is the default sleep interval after any replication request is issued
     */
    private static final int DEFAULT_SLEEP_INTERVAL = 10000;

    private int m_sleepInterval = DEFAULT_SLEEP_INTERVAL;

    private boolean m_activate;

    private Worker m_worker;

    private SipxServiceManager m_sipxServiceManager;

    private String m_serviceBeanId;

    public abstract ServiceConfigurator getServiceConfigurator();

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    public void setSleepInterval(int sleepInterval) {
        m_sleepInterval = sleepInterval;
    }

    @Required
    public void setServiceBeanId(String serviceBeanId) {
        m_serviceBeanId = serviceBeanId;
    }

    @Override
    public void activateConfigFiles() {
        m_activate = true;
        notifyWorker();
    }

    public void activateIfNeeded() {
        if (getActivate()) {
            SipxService service = m_sipxServiceManager.getServiceByBeanId(m_serviceBeanId);
            getServiceConfigurator().replicateServiceConfig(service, true);
        }
    }

    public void init() {
        m_worker = new Worker();
        m_worker.start();
    }

    private void notifyWorker() {
        m_worker.workScheduled();
        LOG.debug("Notify work scheduled");
        synchronized (this) {
            notify();
        }
    }

    private synchronized void waitForWork() throws InterruptedException {
        if (!m_activate) {
            LOG.debug("Waiting for work");
            wait();
            LOG.debug("Work received");
        }
    }

    private synchronized boolean getActivate() {
        boolean activate = m_activate;
        m_activate = false;
        return activate;
    }

    /**
     * Worker thread: waits till something needs to be done, sleeps for a bit and does it.
     */
    private class Worker extends LazyDaemon {
        public Worker() {
            super("activate config worker", m_sleepInterval);
        }

        @Override
        protected void waitForWork() throws InterruptedException {
            LazyConfigFileActivationManager.this.waitForWork();
        }

        @Override
        protected boolean work() {
            activateIfNeeded();
            return true;
        }
    }

}
