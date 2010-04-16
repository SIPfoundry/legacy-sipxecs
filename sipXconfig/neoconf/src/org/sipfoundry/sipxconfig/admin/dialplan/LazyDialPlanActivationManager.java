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

import java.util.Collection;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.common.LazyDaemon;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class LazyDialPlanActivationManager implements DialPlanActivationManager, ApplicationListener {
    private static final Log LOG = LogFactory.getLog(LazyDialPlanActivationManager.class);

    /**
     * 30s is the default sleep interval after any replication request is issued
     */
    private static final int DEFAULT_SLEEP_INTERVAL = 30000;

    private int m_sleepInterval = DEFAULT_SLEEP_INTERVAL;

    private boolean m_replicate;

    private boolean m_restart;

    private DialPlanActivationManager m_target;

    private Worker m_worker;

    private SipxProcessContext m_sipxProcessContext;

    private SbcDeviceManager m_sbcDeviceManager;

    private ProfileManager m_sbcProfileManager;

    public synchronized void replicateDialPlan(boolean restartSbcDevices) {
        m_replicate = true;
        m_restart = m_restart || restartSbcDevices;
        m_sipxProcessContext.markDialPlanRelatedServicesForRestart(SipxProxyService.BEAN_ID,
                    SipxRegistrarService.BEAN_ID);
        notifyWorker();
    }

    public void replicateIfNeeded() {
        if (getReplicate()) {
            boolean restart = getRestart();
            m_target.replicateDialPlan(restart);
        }
    }

    @Override
    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof DialPlanActivatedEvent) {
            getReplicate();
            if (getRestart() && !((DialPlanActivatedEvent) event).isSbcsRestarted()) {
                Collection<Integer> sbcIds = m_sbcDeviceManager.getAllSbcDeviceIds();
                m_sbcProfileManager.restartDevices(sbcIds, null);
            }
        }
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
        m_restart = false;
        return restart;
    }

    private synchronized boolean getReplicate() {
        boolean replicate = m_replicate;
        m_replicate = false;
        return replicate;
    }

    /**
     * Worker thread: waits till something needs to be done, sleeps for a bit and does it.
     */
    private class Worker extends LazyDaemon {
        public Worker() {
            super("dial plan worker", m_sleepInterval);
        }

        @Override
        protected void waitForWork() throws InterruptedException {
            LazyDialPlanActivationManager.this.waitForWork();
        }

        @Override
        protected boolean work() {
            replicateIfNeeded();
            return true;
        }
    }

    @Required
    public void setTarget(DialPlanActivationManager target) {
        m_target = target;
    }

    @Required
    public void setSipxProcessContext(SipxProcessContext sipxProcessContext) {
        m_sipxProcessContext = sipxProcessContext;
    }

    @Required
    public void setSbcDeviceManager(SbcDeviceManager sbcDeviceManager) {
        m_sbcDeviceManager = sbcDeviceManager;
    }

    @Required
    public void setSbcProfileManager(ProfileManager sbcProfileManager) {
        m_sbcProfileManager = sbcProfileManager;
    }

    public void setSleepInterval(int sleepInterval) {
        m_sleepInterval = sleepInterval;
    }
}
