/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import org.sipfoundry.sipxconfig.common.LazyDaemon;

public class LazyProfileManagerImpl implements ProfileManager {
    private Set m_ids = new HashSet();

    private ProfileManager m_target;

    private int m_sleepInterval;

    private Worker m_worker;

    public void setSleepInterval(int sleepInterval) {
        m_sleepInterval = sleepInterval;
    }

    public void setTarget(ProfileManager target) {
        m_target = target;
    }

    public synchronized void generateProfilesAndRestart(Collection phoneIds) {
        m_ids.addAll(phoneIds);
        m_worker.workScheduled();
        notify();
    }

    public synchronized void generateProfileAndRestart(Integer phoneId) {
        m_ids.add(phoneId);
        m_worker.workScheduled();
        notify();
    }

    private synchronized void waitForWork() throws InterruptedException {
        if (m_ids.isEmpty()) {
            wait();
        }
    }

    public void init() {
        m_worker = new Worker();
        m_worker.start();
    }

    private synchronized Set getTasks() {
        if (m_ids.isEmpty()) {
            return Collections.EMPTY_SET;
        }
        Set oldTasks = m_ids;
        m_ids = new HashSet();
        return oldTasks;
    }

    private class Worker extends LazyDaemon {
        public Worker() {
            super("Profile Manager thread", m_sleepInterval);
        }

        protected void waitForWork() throws InterruptedException {
            LazyProfileManagerImpl.this.waitForWork();
        }

        protected boolean work() {
            m_target.generateProfilesAndRestart(getTasks());
            return true;
        }
    }
}
