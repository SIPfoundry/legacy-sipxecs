/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.admin.LoggingManager;
import org.sipfoundry.sipxconfig.setting.ValueStorage;

public class SipxSupervisorService extends SipxService implements LoggingEntity {
    public static final String BEAN_ID = "sipxSupervisorService";
    private static final String LOG_LEVEL_PATH = "log_level";
    private LoggingManager m_loggingManager;

    @Override
    public String getLogLevel() {
        ValueStorage storage = (ValueStorage) getInitializeValueStorage();
        String logLevel = storage.getSettingValue(LOG_LEVEL_PATH);
        return logLevel == null ? "INFO" : logLevel;
    }

    @Override
    public void setLogLevel(String logLevel) {
        if (logLevel != null) {
            ValueStorage storage = (ValueStorage) getInitializeValueStorage();
            String oldLogLevel = storage.getSettingValue(LOG_LEVEL_PATH);
            if (!logLevel.equals(oldLogLevel)) {
                m_loggingManager.getEntitiesToProcess().add(this);
                storage.setSettingValue(LOG_LEVEL_PATH, logLevel);
                getSipxServiceManager().storeService(this);
            }
        }
    }

    @Override
    public String getLabelKey() {
        return "label.sipxSupervisorService";
    }

    @Override
    public String getLogSetting() {
        return LOG_LEVEL_PATH;
    }

    @Override
    public void setLoggingManager(LoggingManager loggingManager) {
        m_loggingManager = loggingManager;
    }
}
