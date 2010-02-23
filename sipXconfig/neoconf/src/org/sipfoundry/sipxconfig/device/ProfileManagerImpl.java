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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.springframework.beans.factory.annotation.Required;

public class ProfileManagerImpl implements ProfileManager {
    private static final Log LOG = LogFactory.getLog(ProfileManagerImpl.class);

    private JobContext m_jobContext;

    private RestartManager m_restartManager;

    private DeviceSource m_deviceSource;

    public final void generateProfiles(Collection<Integer> devices, boolean restart, Date restartTime) {
        for (Integer id : devices) {
            generateProfile(id, restart, restartTime);
        }
    }

    public final void generateProfile(Integer id, boolean restart, Date restartTime) {
        Device d = m_deviceSource.loadDevice(id);
        generate(d);
        if (restart) {
            m_restartManager.restart(id, restartTime);
        }
    }

    protected void generate(Device d) {
        if (d.getModel().isProjectionSupported()) {
            String jobName = "Projection for: " + d.getNiceName();
            Serializable jobId = m_jobContext.schedule(jobName);
            try {
                m_jobContext.start(jobId);
                ProfileLocation location = d.getProfileLocation();
                d.generateProfiles(location);
                m_jobContext.success(jobId);
            } catch (RuntimeException e) {
                m_jobContext.failure(jobId, null, e);
                // do not throw error, job queue will stop running.
                // error gets logged to job error table and sipxconfig.log
                LOG.error(e);
            }
        }
    }

    @Required
    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }

    @Required
    public void setRestartManager(RestartManager restartManager) {
        m_restartManager = restartManager;
    }

    @Required
    public void setDeviceSource(DeviceSource deviceSource) {
        m_deviceSource = deviceSource;
    }
}
