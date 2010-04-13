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
    public static final String SIP_PORT_SETTING = "proxy-configuration/SIP_PORT";
    public static final String SIP_SECURE_PORT_SETTING = "proxy-configuration/TLS_SIP_PORT";

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

    @Override
    public String getSipPort() {
        return getSettingValue(SIP_PORT_SETTING);
    }

    @Override
    public void setSipPort(String sipPort) {
        setSettingValue(SIP_PORT_SETTING, sipPort);
    }

    public String getSecureSipPort() {
        return getSettingValue(SIP_SECURE_PORT_SETTING);
    }

    public String getSipTCPPort() {
        return getSipPort();
    }

    public String getSipUDPPort() {
        return getSipPort();
    }
}
