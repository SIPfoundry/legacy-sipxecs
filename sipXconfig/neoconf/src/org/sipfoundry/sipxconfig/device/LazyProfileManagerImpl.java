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
import java.util.Date;
import java.util.HashMap;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.LazyDaemon;

public class LazyProfileManagerImpl implements ProfileManager {
    private Map<Integer, RestartInfo> m_ids = new HashMap<Integer, RestartInfo>();

    private ProfileManager m_target;

    private int m_sleepInterval;

    private Worker m_worker;

    public void setSleepInterval(int sleepInterval) {
        m_sleepInterval = sleepInterval;
    }

    public void setTarget(ProfileManager target) {
        m_target = target;
    }

    public synchronized void generateProfiles(Collection<Integer> phoneIds, boolean restart, Date restartTime) {
        for (Integer id : phoneIds) {
            updateRestart(id, restart, restartTime);
        }
        m_worker.workScheduled();
        notify();
    }

    public synchronized void generateProfile(Integer phoneId, boolean restart, Date restartTime) {
        updateRestart(phoneId, restart, restartTime);
        m_worker.workScheduled();
        notify();
    }

    private void updateRestart(Integer id, boolean restart, Date restartTime) {
        m_ids.put(id, new RestartInfo(restart, restartTime));
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

    private synchronized Map<Integer, RestartInfo> getTasks() {
        if (m_ids.isEmpty()) {
            return Collections.emptyMap();
        }
        Map<Integer, RestartInfo> oldTasks = m_ids;
        m_ids = new HashMap<Integer, RestartInfo>();
        return oldTasks;
    }

    private class Worker extends LazyDaemon {
        public Worker() {
            super("Profile Manager thread", m_sleepInterval);
        }

        @Override
        protected void waitForWork() throws InterruptedException {
            LazyProfileManagerImpl.this.waitForWork();
        }

        @Override
        protected boolean work() {
            Map<Integer, RestartInfo> tasks = getTasks();
            for (Map.Entry<Integer, RestartInfo> entry : tasks.entrySet()) {
                RestartInfo ri = entry.getValue();
                m_target.generateProfile(entry.getKey(), ri.isRestart(), ri.getTime());
            }
            return true;
        }
    }

    private static class RestartInfo {
        private final boolean m_restart;
        private final Date m_time;

        public RestartInfo(boolean restart, Date time) {
            m_restart = restart;
            m_time = time;
        }

        public Date getTime() {
            return m_time;
        }

        public boolean isRestart() {
            return m_restart;
        }
    }
}
