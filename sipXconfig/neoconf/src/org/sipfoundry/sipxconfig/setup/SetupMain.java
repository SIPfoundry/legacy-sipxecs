/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setup;

import java.util.List;

import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.SystemTaskEntryPoint;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.job.Job;
import org.sipfoundry.sipxconfig.job.JobContext;

public class SetupMain implements SystemTaskEntryPoint {
    private ConfigManager m_configManager;
    private SetupManager m_setupManager;
    private JobContext m_jobContext;

    @Override
    public void runSystemTask(String[] args) {
        m_setupManager.setup();
        m_configManager.runProviders();
        List<Job> failed = m_jobContext.getFailedJobs();
        if (failed != null && failed.size() > 0) {
            Job job = failed.get(0);
            Throwable e = job.getException();
            if (e != null) {
                if (e instanceof RuntimeException) {
                    throw (RuntimeException) e;
                }
                throw new UserException("Failure setting up system", e);
            } else {
                throw new UserException(job.getErrorMsg());
            }
        }
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public void setSetupManager(SetupManager setupManager) {
        m_setupManager = setupManager;
    }

    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }
}
