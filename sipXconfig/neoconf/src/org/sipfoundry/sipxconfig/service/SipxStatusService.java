/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxStatusService extends SipxService implements LoggingEntity {
    public static final String BEAN_ID = "sipxStatusService";

    public static final String LOG_SETTING = "status-config/SIP_STATUS_LOG_LEVEL";

    private int m_httpsPort;

    public void setHttpsPort(int httpsPort) {
        m_httpsPort = httpsPort;
    }

    public int getHttpsPort() {
        return m_httpsPort;
    }

    @Override
    public String getSipPort() {
        Setting statusSettings = getSettings().getSetting("status-config");
        return statusSettings.getSetting("SIP_STATUS_SIP_PORT").getValue();
    }

    @Override
    public String getLogSetting() {
        return LOG_SETTING;
    }

    @Override
    public void setLogLevel(String logLevel) {
        super.setLogLevel(logLevel);
    }

    @Override
    public String getLogLevel() {
        return super.getLogLevel();
    }

    @Override
    public String getLabelKey() {
        return super.getLabelKey();
    }
}
