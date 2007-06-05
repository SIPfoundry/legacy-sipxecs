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
import java.util.HashMap;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.LazyDaemon;

public class LazyProfileManagerImpl implements ProfileManager {
    private Map<Integer, Boolean> m_ids = new HashMap<Integer, Boolean>();

    private ProfileManager m_target;

    private int m_sleepInterval;

    private Worker m_worker;

    public void setSleepInterval(int sleepInterval) {
        m_sleepInterval = sleepInterval;
    }

    public void setTarget(ProfileManager target) {
        m_target = target;
    }

    public synchronized void generateProfiles(Collection<Integer> phoneIds, boolean restart) {
        for (Integer id : phoneIds) {
            updateRestart(id, restart);
        }
        m_worker.workScheduled();
        notify();
    }

    public synchronized void generateProfile(Integer phoneId, boolean restart) {
        updateRestart(phoneId, restart);
        m_worker.workScheduled();
        notify();
    }

    private void updateRestart(Integer id, boolean restart) {
        boolean newRestart = restart;
        if (m_ids.containsKey(id)) {
            newRestart = restart || m_ids.get(id);
        }
        m_ids.put(id, newRestart);
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

    private synchronized Map<Integer, Boolean> getTasks() {
        if (m_ids.isEmpty()) {
            return Collections.emptyMap();
        }
        Map<Integer, Boolean> oldTasks = m_ids;
        m_ids = new HashMap<Integer, Boolean>();
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
            Map<Integer, Boolean> tasks = getTasks();
            for (Map.Entry<Integer, Boolean> entry : tasks.entrySet()) {
                m_target.generateProfile(entry.getKey(), entry.getValue());
            }
            return true;
        }
    }
}
