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

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.commons.util.IPAddressUtil;
import org.sipfoundry.sipxconfig.ConfigurationFile;
import org.sipfoundry.sipxconfig.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.beans.factory.annotation.Required;

public class SipxProxyService extends SipxService implements LoggingEntity {

    public static final String BEAN_ID = "sipxProxyService";

    public static final String LOG_SETTING = "proxy-configuration/SIPX_PROXY_LOG_LEVEL";
    public static final String SIP_PORT_SETTING = "proxy-configuration/SIP_PORT";
    public static final String SIP_SECURE_PORT_SETTING = "proxy-configuration/TLS_SIP_PORT";
    public static final String ENABLE_BRIDGE_PROXY_RELAY = "proxy-configuration/ENABLE_BRIDGE_PROXY_RELAY";
    public static final String AUTOBAN_THRESHOLD_VIOLATORS = "call-rate-limit/SIPX_PROXY_AUTOBAN_THRESHOLD_VIOLATORS";
    public static final String ALLOWED_PACKETS_PER_SECOND = "call-rate-limit/SIPX_PROXY_PACKETS_PER_SECOND_THRESHOLD";
    public static final String THRESHOLD_VIOLATION_RATE = "call-rate-limit/SIPX_PROXY_THRESHOLD_VIOLATION_RATE";
    public static final String BAN_LIFETIME = "call-rate-limit/SIPX_PROXY_BAN_LIFETIME";
    public static final String WHITE_LIST = "call-rate-limit/SIPX_PROXY_WHITE_LIST";
    public static final String BLACK_LIST = "call-rate-limit/SIPX_PROXY_BLACK_LIST";

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

    public String getSipTLSPort() {
        return getSecureSipPort();
    }

    public boolean isEnabledBridgeProxyRelay() {
        return (Boolean) getSettingTypedValue(ENABLE_BRIDGE_PROXY_RELAY);
    }

    public boolean getAutobanThresholdViolators() {
        return (Boolean) getSettingTypedValue(AUTOBAN_THRESHOLD_VIOLATORS);
    }

    public Integer getAllowedPacketsPerSecond() {
        return (Integer) getSettingTypedValue(ALLOWED_PACKETS_PER_SECOND);
    }

    public Integer getThresholdViolationRate() {
        return (Integer) getSettingTypedValue(THRESHOLD_VIOLATION_RATE);
    }

    public Integer getBanLifetime() {
        return (Integer) getSettingTypedValue(BAN_LIFETIME);
    }

    public String getWhiteList() {
        return (String) getSettingTypedValue(WHITE_LIST);
    }

    public String getBlackList() {
        return (String) getSettingTypedValue(BLACK_LIST);
    }

    @Override
    public void validate() {
        checkIpList(getWhiteList(), "&msg.invalidWhiteList");
        checkIpList(getBlackList(), "&msg.invalidBlackList");
    }

    private void checkIpList(String list, String message) {
        if (StringUtils.isNotEmpty(list)) {
            String[] ipTokens = StringUtils.split(list, ',');
            for (String ip : ipTokens) {
                if (!IPAddressUtil.isLiteralIPAddress(StringUtils.trim(ip))
                        && !IPAddressUtil.isLiteralIPSubnetAddress(StringUtils.trim(ip))) {
                    throw new UserException(message, ip);
                }
            }
        }
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
