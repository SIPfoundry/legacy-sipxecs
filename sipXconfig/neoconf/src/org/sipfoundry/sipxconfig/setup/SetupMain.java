/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setup;

import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.SystemTaskEntryPoint;
import org.sipfoundry.sipxconfig.job.JobContext;

public class SetupMain implements SystemTaskEntryPoint {
    private ConfigManager m_configManager;
    private SetupManager m_setupManager;
    private JobContext m_jobContext;

    @Override
    public void runSystemTask(String[] args) {
        m_setupManager.setup(SetupManager.Context.SETUP_MAIN);
        m_configManager.runProviders();
        m_jobContext.checkAndThrowErrorOnFailedJobs();
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
