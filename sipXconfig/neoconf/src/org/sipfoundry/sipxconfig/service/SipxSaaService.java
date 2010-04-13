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

public class SipxSaaService extends SipxService implements LoggingEntity {
    public static final String BEAN_ID = "sipxSaaService";

    public static final String LOG_SETTING = "logging/SIP_SAA_LOG_LEVEL";

    private int m_tcpPort;

    private int m_udpPort;

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

    public int getTcpPort() {
        return m_tcpPort;
    }

    public void setTcpPort(int tcpPort) {
        m_tcpPort = tcpPort;
    }

    public int getUdpPort() {
        return m_udpPort;
    }

    public void setUdpPort(int udpPort) {
        m_udpPort = udpPort;
    }
}
