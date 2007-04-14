/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import java.io.Serializable;
import java.util.Collection;
import java.util.Iterator;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.device.RestartManager;
import org.sipfoundry.sipxconfig.job.JobContext;

public class RestartManagerImpl implements RestartManager {
    private static final Log LOG = LogFactory.getLog(RestartManagerImpl.class);

    private static final int DEFAULT_THROTTLE_INTERVAL = 1000;

    private JobContext m_jobContext;

    private PhoneContext m_phoneContext;

    /**
     * The number of millis by which we are going to delay restarting a phone if more than one
     * phone is restarted. We are delaying restart of the phone. The default value is 1 second,
     * but it can be changes by modifying sipxcongig.properties.in file. Set to 0 to remove the
     * throttle.
     * 
     */
    private int m_throttleInterval = DEFAULT_THROTTLE_INTERVAL;

    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public void setThrottleInterval(int throttleInterval) {
        m_throttleInterval = throttleInterval;
    }

    public void restart(Collection phoneIds) {
        for (Iterator iter = phoneIds.iterator(); iter.hasNext();) {
            Integer id = (Integer) iter.next();
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

    public void restart(Integer phoneId) {
        Phone phone = m_phoneContext.loadPhone(phoneId);
        restart(phone);
    }

    private void restart(Phone phone) {
        Serializable jobId = m_jobContext.schedule("Restarting phone " + phone.getSerialNumber());
        try {
            m_jobContext.start(jobId);
            phone.restart();
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
