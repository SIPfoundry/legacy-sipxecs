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

import java.io.Serializable;
import java.util.Collection;
import java.util.Date;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.springframework.beans.factory.annotation.Required;

public class ScheduledRestartManagerImpl implements RestartManager {
    private static final Log LOG = LogFactory.getLog(ScheduledRestartManagerImpl.class);

    private static final int DEFAULT_THROTTLE_INTERVAL = 1000;

    private JobContext m_jobContext;

    private DeviceSource m_deviceSource;

    private ScheduledExecutorService m_executorService;

    /**
     * The number of millis by which we are going to delay restarting a device if more than one
     * device is restarted. We are delaying restart of the device. The default value is 1 second,
     * but it can be changes by modifying sipxconfig.properties file. Set to 0 to remove the
     * throttle.
     *
     */
    private int m_throttleInterval = DEFAULT_THROTTLE_INTERVAL;

    /**
     * Initial wait interval.
     */
    private int m_sleepInterval;

    public void restart(Collection<Integer> deviceIds, Date scheduleTime) {
        long delay = calculateDelay(scheduleTime);
        for (Integer id : deviceIds) {
            Device device = m_deviceSource.loadDevice(id);
            if (device.getModel().isRestartSupported()) {
                RestartTask task = new RestartTask(id);
                m_executorService.schedule(task, delay, TimeUnit.MILLISECONDS);
                delay += m_throttleInterval;
            }
        }
    }

    long calculateDelay(Date scheduleTime) {
        if (scheduleTime == null) {
            return m_sleepInterval;
        }
        return scheduleTime.getTime() - System.currentTimeMillis() + m_sleepInterval;
    }

    public void restart(Integer deviceId, Date scheduleTime) {
        Device device = m_deviceSource.loadDevice(deviceId);
        if (device.getModel().isRestartSupported()) {
            RestartTask task = new RestartTask(deviceId);
            long delay = calculateDelay(scheduleTime);
            m_executorService.schedule(task, delay, TimeUnit.MILLISECONDS);
        }
    }

    @Required
    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }

    @Required
    public void setDeviceSource(DeviceSource deviceSource) {
        m_deviceSource = deviceSource;
    }

    @Required
    public void setExecutorService(ScheduledExecutorService executorService) {
        m_executorService = executorService;
    }

    public void setThrottleInterval(int throttleInterval) {
        m_throttleInterval = throttleInterval;
    }

    public void setSleepInterval(int sleepInterval) {
        m_sleepInterval = sleepInterval;
    }

    private class RestartTask implements Runnable {
        private final int m_deviceId;

        public RestartTask(int deviceId) {
            m_deviceId = deviceId;
        }

        public void run() {
            Device device = m_deviceSource.loadDevice(m_deviceId);
            String jobName = "Restarting: " + device.getNiceName();
            Serializable jobId = m_jobContext.schedule(jobName);
            try {
                m_jobContext.start(jobId);
                device.restart();
                m_jobContext.success(jobId);
            } catch (RestartException e) {
                m_jobContext.failure(jobId, null, e);
            } catch (RuntimeException e) {
                m_jobContext.failure(jobId, null, e);
                // do not throw error, job queue will stop running.
                // error gets logged to job error table and sipxconfig.log
                LOG.error(e);
            }
        }
    }
}
