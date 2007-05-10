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
import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.device.RestartManager;
import org.sipfoundry.sipxconfig.job.JobContext;

public class ProfileManagerImpl implements ProfileManager {

    private static final Log LOG = LogFactory.getLog(ProfileManagerImpl.class);

    private JobContext m_jobContext;

    private PhoneContext m_phoneContext;

    private RestartManager m_restartManager;

    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public void setRestartManager(RestartManager restartManager) {
        m_restartManager = restartManager;
    }

    public void generateProfilesAndRestart(Collection phones) {
        for (Iterator iter = phones.iterator(); iter.hasNext();) {
            Integer id = (Integer) iter.next();
            generateProfileAndRestart(id);
        }
    }

    public void generateProfileAndRestart(Integer phoneId) {
        Phone phone = m_phoneContext.loadPhone(phoneId);
        generate(phone);
        m_restartManager.restart(phoneId);
    }

    private void generate(Phone phone) {
        Serializable jobId = m_jobContext.schedule("Projection for phone "
                + phone.getSerialNumber());
        try {
            m_jobContext.start(jobId);
            ProfileLocation location = phone.getModel().getDefaultProfileLocation();
            phone.generateProfiles(location);
            m_jobContext.success(jobId);
        } catch (RuntimeException e) {
            m_jobContext.failure(jobId, null, e);
            // do not throw error, job queue will stop running.
            // error gets logged to job error table and sipxconfig.log
            LOG.error(e);
        }
    }
}
