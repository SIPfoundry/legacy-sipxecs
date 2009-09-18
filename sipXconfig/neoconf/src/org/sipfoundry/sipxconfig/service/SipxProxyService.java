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

public class SipxProxyService extends SipxService implements LoggingEntity {

    public static final String BEAN_ID = "sipxProxyService";

    public static final String LOG_SETTING = "proxy-configuration/SIPX_PROXY_LOG_LEVEL";

    private String m_secureSipPort;

    public String getSecureSipPort() {
        return m_secureSipPort;
    }

    public void setSecureSipPort(String secureSipPort) {
        this.m_secureSipPort = secureSipPort;
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
