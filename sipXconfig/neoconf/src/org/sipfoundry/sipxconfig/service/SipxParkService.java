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

public class SipxParkService extends SipxService implements LoggingEntity {
    public static final String BEAN_ID = "sipxParkService";

    public static final String LOG_SETTING = "park-config/SIP_PARK_LOG_LEVEL";

    private String m_parkServerSipPort;

    public String getParkServerSipPort() {
        return m_parkServerSipPort;
    }

    public void setParkServerSipPort(String parkServerSipPort) {
        m_parkServerSipPort = parkServerSipPort;
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
