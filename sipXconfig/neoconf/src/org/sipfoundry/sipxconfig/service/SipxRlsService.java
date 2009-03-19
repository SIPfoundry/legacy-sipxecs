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

public class SipxRlsService extends SipxService implements LoggingEntity {
    public static final String BEAN_ID = "sipxRlsService";

    public static final String LOG_SETTING = "rls-config/SIP_RLS_LOG_LEVEL";

    private String m_rlsPort;

    private String m_logLevel;

    public String getRlsPort() {
        return m_rlsPort;
    }

    public void setRlsPort(String rlsPort) {
        m_rlsPort = rlsPort;
    }

    public String getLogSetting() {
        return LOG_SETTING;
    }

    public void setLogLevel(String logLevel) {
        super.setLogLevel(logLevel);
    }

    public String getLogLevel() {
        return super.getLogLevel();
    }

    public String getLabelKey() {
        return super.getLabelKey();
    }
}
