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

    private static enum TaskInfo { GENERATE, GENERATE_RESTART, RESTART };

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
            updateRestart(id, restart ? TaskInfo.GENERATE : TaskInfo.GENERATE_RESTART, restartTime);
        }
        m_worker.workScheduled();
        notify();
    }

    public synchronized void generateProfile(Integer phoneId, boolean restart, Date restartTime) {
        updateRestart(phoneId, restart ? TaskInfo.GENERATE : TaskInfo.GENERATE_RESTART, restartTime);
        m_worker.workScheduled();
        notify();
    }

    @Override
    public synchronized void restartDevices(Collection<Integer> phoneIds, Date restartTime) {
        for (Integer id : phoneIds) {
            updateRestart(id, TaskInfo.RESTART, restartTime);
        }
        m_worker.workScheduled();
        notify();
    }

    @Override
    public synchronized void restartDevice(Integer phoneId, Date restartTime) {
        updateRestart(phoneId, TaskInfo.RESTART, restartTime);
        m_worker.workScheduled();
        notify();
    }

    private void updateRestart(Integer id, TaskInfo taskInfo, Date restartTime) {
        m_ids.put(id, new RestartInfo(taskInfo, restartTime));
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
                if (ri.isRestartOnly()) {
                    m_target.restartDevice(entry.getKey(), ri.getTime());
                } else {
                    m_target.generateProfile(entry.getKey(), ri.isRestart(), ri.getTime());
                }
            }
            return true;
        }
    }

    private static class RestartInfo {
        private final TaskInfo m_taskInfo;
        private final Date m_time;

        public RestartInfo(TaskInfo taskInfo, Date time) {
            m_taskInfo = taskInfo;
            m_time = time;
        }

        public Date getTime() {
            return m_time;
        }

        public boolean isRestart() {
            return m_taskInfo.equals(TaskInfo.GENERATE_RESTART);
        }

        public boolean isRestartOnly() {
            return m_taskInfo.equals(TaskInfo.RESTART);
        }
    }
}
