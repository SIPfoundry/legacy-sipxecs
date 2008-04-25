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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.springframework.beans.factory.annotation.Required;

public class RestartManagerImpl implements RestartManager {
    private static final Log LOG = LogFactory.getLog(RestartManagerImpl.class);

    private static final int DEFAULT_THROTTLE_INTERVAL = 1000;

    private JobContext m_jobContext;

    private DeviceSource m_deviceSource;

    /**
     * The number of millis by which we are going to delay restarting a device if more than one
     * device is restarted. We are delaying restart of the device. The default value is 1 second,
     * but it can be changes by modifying sipxcongig.properties.in file. Set to 0 to remove the
     * throttle.
     * 
     */
    private int m_throttleInterval = DEFAULT_THROTTLE_INTERVAL;

    public void restart(Collection<Integer> deviceIds) {
        for (Integer id : deviceIds) {
            restart(id);
            throttle();
        }
    }

    private void throttle() {
        if (m_throttleInterval <= 0) {
            return;
        }
        try {
            Thread.sleep(m_throttleInterval);
        } catch (InterruptedException e) {
            LOG.error("Ignoring exception", e);
        }
    }

    public void restart(Integer deviceId) {
        Device device = m_deviceSource.loadDevice(deviceId);
        restart(device);
    }

    private void restart(Device device) {
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

    @Required
    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }

    @Required
    public void setDeviceSource(DeviceSource deviceSource) {
        m_deviceSource = deviceSource;
    }

    public void setThrottleInterval(int throttleInterval) {
        m_throttleInterval = throttleInterval;
    }
}
