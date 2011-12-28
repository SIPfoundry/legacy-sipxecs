/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.proxy;


import java.util.Collection;
import java.util.Collections;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.commons.util.IPAddressUtil;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingsValidator;

public class ProxySettings extends BeanWithSettings implements DeployConfigOnEdit, SettingsValidator {
    public static final String LOG_SETTING = "proxy-configuration/SIPX_PROXY_LOG_LEVEL";
    public static final String SIP_PORT_SETTING = "proxy-configuration/SIPX_PROXY_TCP_PORT";
    public static final String SIP_UDP_PORT_SETTING = "proxy-configuration/SIPX_PROXY_UDP_PORT";
    public static final String SIP_SECURE_PORT_SETTING = "proxy-configuration/TLS_SIP_PORT";
    public static final String AUTOBAN_THRESHOLD_VIOLATORS = "call-rate-limit/SIPX_PROXY_AUTOBAN_THRESHOLD_VIOLATORS";
    public static final String ALLOWED_PACKETS_PER_SECOND = "call-rate-limit/SIPX_PROXY_PACKETS_PER_SECOND_THRESHOLD";
    public static final String THRESHOLD_VIOLATION_RATE = "call-rate-limit/SIPX_PROXY_THRESHOLD_VIOLATION_RATE";
    public static final String BAN_LIFETIME = "call-rate-limit/SIPX_PROXY_BAN_LIFETIME";
    public static final String WHITE_LIST = "call-rate-limit/SIPX_PROXY_WHITE_LIST";
    public static final String BLACK_LIST = "call-rate-limit/SIPX_PROXY_BLACK_LIST";

    public int getSipTcpPort() {
        return (Integer) getSettingTypedValue(SIP_PORT_SETTING);
    }

    public int getSipUdpPort() {
        return (Integer) getSettingTypedValue(SIP_UDP_PORT_SETTING);
    }

    public int getSecureSipPort() {
        return (Integer) getSettingTypedValue(SIP_SECURE_PORT_SETTING);
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

    public int getDefaultInitDelay() {
        return (Integer) getSettingTypedValue("proxy-configuration/SIPX_PROXY_DEFAULT_SERIAL_EXPIRES");
    }

    @Override
    public void validate(Setting settings) {
        ProxySettings p = (ProxySettings) settings;
        checkIpList(p.getWhiteList(), "&msg.invalidWhiteList");
        checkIpList(p.getBlackList(), "&msg.invalidBlackList");
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

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxproxy/sipxproxy.xml");
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) ProxyManager.FEATURE);
    }
}
