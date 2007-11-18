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
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.job.JobContext;

public abstract class AbstractProfileManager implements ProfileManager {
    private static final Log LOG = LogFactory.getLog(AbstractProfileManager.class);

    private JobContext m_jobContext;

    public final void generateProfiles(Collection<Integer> devices, boolean restart) {
        for (Integer id : devices) {
            generateProfile(id, restart);
        }
    }

    public final void generateProfile(Integer id, boolean restart) {
        Device d = loadDevice(id);
        generate(d);
        if (restart) {
            restartDevice(id);
        }
    }

    protected void generate(Device d) {
        Serializable jobId = m_jobContext.schedule(formatJobName(d));
        try {
            m_jobContext.start(jobId);
            ProfileLocation location = d.getModel().getDefaultProfileLocation();
            d.generateProfiles(location);
            m_jobContext.success(jobId);
        } catch (RuntimeException e) {
            m_jobContext.failure(jobId, null, e);
            // do not throw error, job queue will stop running.
            // error gets logged to job error table and sipxconfig.log
            LOG.error(e);
        }
    }

    /** Overwrite to load device from appropriate context */
    protected abstract Device loadDevice(Integer id);

    /** Called to restart device after successful profile generation */
    protected abstract void restartDevice(Integer id);

    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }

    static String formatJobName(Device device) {
        StringBuilder jn = new StringBuilder("Projection for: ");
        if (device instanceof NamedObject) {
            NamedObject namedDevice = (NamedObject) device;
            jn.append(namedDevice.getName());
            jn.append("/");
        }
        jn.append(device.getSerialNumber());
        return jn.toString();
    }
}
