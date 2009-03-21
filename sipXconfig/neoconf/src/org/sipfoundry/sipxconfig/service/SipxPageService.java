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

public class SipxPageService extends SipxService implements LoggingEntity {

    public static final String BEAN_ID = "sipxPageService";

    public static final String LOG_SETTING = "page-config/SIP_PAGE_LOG_LEVEL";

    private static final String SIP_PORT = "page-config/PAGE_SERVER_SIP_PORT";

    private String m_audioDirectory;

    public String getAudioDir() {
        return m_audioDirectory;
    }

    public void setAudioDir(String audioDirectory) {
        m_audioDirectory = audioDirectory;
    }

    @Override
    public String getSipPort() {
        return getSettingValue(SIP_PORT);
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
