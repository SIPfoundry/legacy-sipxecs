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

import java.util.Collections;

import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.springframework.beans.factory.annotation.Required;

public class SipxProxyService extends SipxService implements LoggingEntity {

    public static final String BEAN_ID = "sipxProxyService";

    public static final String LOG_SETTING = "proxy-configuration/SIPX_PROXY_LOG_LEVEL";
    public static final String SIP_PORT_SETTING = "proxy-configuration/SIP_PORT";
    public static final String SIP_SECURE_PORT_SETTING = "proxy-configuration/TLS_SIP_PORT";
    public static final String ENABLE_BRIDGE_PROXY_RELAY = "proxy-configuration/ENABLE_BRIDGE_PROXY_RELAY";

    private SipxProcessContext m_sipxProcessContext;
    private SipxReplicationContext m_replicationContext;
    private ConfigurationFile m_sipxbridgeConfig;

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

    public void setSecureSipPort(String secureSipPort) {
        setSettingValue(SIP_SECURE_PORT_SETTING, secureSipPort);
    }

    public String getSipTCPPort() {
        return getSipPort();
    }

    public String getSipUDPPort() {
        return getSipPort();
    }

    public boolean isEnabledBridgeProxyRelay() {
        return (Boolean) getSettingTypedValue(ENABLE_BRIDGE_PROXY_RELAY);
    }

    public void onConfigChange() {
        SipxService bridgeService = getSipxServiceManager().getServiceByBeanId(SipxBridgeService.BEAN_ID);
        m_sipxProcessContext.markServicesForRestart(Collections.singleton(bridgeService));
        m_replicationContext.replicate(m_sipxbridgeConfig);
    }

    @Required
    public void setSipxProcessContext(SipxProcessContext sipxProcessContext) {
        m_sipxProcessContext = sipxProcessContext;
    }

    @Required
    public void setSipxReplicationContext(SipxReplicationContext context) {
        m_replicationContext = context;
    }

    @Required
    public void setSbcSipXbridgeConfiguration(ConfigurationFile config) {
        m_sipxbridgeConfig = config;
    }

}
