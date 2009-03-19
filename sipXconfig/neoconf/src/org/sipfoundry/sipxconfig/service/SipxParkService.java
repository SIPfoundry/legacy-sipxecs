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

    private String m_logLevel;

    public String getParkServerSipPort() {
        return m_parkServerSipPort;
    }

    public void setParkServerSipPort(String parkServerSipPort) {
        m_parkServerSipPort = parkServerSipPort;
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
