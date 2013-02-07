/**
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.firewall;

import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.commons.util.IPAddressUtil;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class FirewallSettings extends PersistableSettings implements DeployConfigOnEdit {
    private boolean m_unmanagedDefault;

    public FirewallSettings() {
        addDefaultBeanSettingHandler(new Defaults());
    }

    public class Defaults {
        @SettingEntry(path = "sys/unmanaged")
        public boolean unmanaged() {
            return m_unmanagedDefault;
        }
    }

    public void validate() {
        validateIpList(getWhiteList());
        validateIpList(getBlackList());
    }

    private void validateIpList(String ipList) {
        if (!IPAddressUtil.validateIpList(ipList)) {
            throw new UserException("&msg.invalidcidr", ipList);
        }
    }

    @Override
    public String getBeanId() {
        return "firewallSettings";
    }

    public Setting getSystemSettings() {
        return getSettings().getSetting("sys");
    }

    public String getBlackList() {
        return (String) getSettingTypedValue("dos/black_list");
    }

    public String getWhiteList() {
        return (String) getSettingTypedValue("dos/white_list");
    }

    public boolean isLogDroppedPacketsEnabled() {
        return (Boolean) getSettingTypedValue("logging/enable-drop");
    }

    public boolean isLogDosPacketsEnabled() {
        return (Boolean) getSettingTypedValue("logging/enable-dos");
    }

    public boolean isLogRatePacketsEnabled() {
        return (Boolean) getSettingTypedValue("logging/enable-ratelimit");
    }

    public boolean isLogSipRegisterEnabled() {
        return (Boolean) getSettingTypedValue("logging/sip/enable-register");
    }

    public boolean isLogSipInviteEnabled() {
        return (Boolean) getSettingTypedValue("logging/sip/enable-invite");
    }

    public boolean isLogSipAckEnabled() {
        return (Boolean) getSettingTypedValue("logging/sip/enable-ack");
    }

    public boolean isLogSipOptionsEnabled() {
        return (Boolean) getSettingTypedValue("logging/sip/enable-options");
    }

    public boolean isLogSipSubscribeEnabled() {
        return (Boolean) getSettingTypedValue("logging/sip/enable-subscribe");
    }

    public int getLogLimitNumber() {
        return (Integer) getSettingTypedValue("logging/limit-drop-no");
    }

    public String getLogLimitInterval() {
        return (String) getSettingTypedValue("logging/limit-drop-time");
    }

    public Set<String> getBlackListSet() {
        return IPAddressUtil.getIpsSet(getBlackList());
    }

    public Set<String> getWhiteListSet() {
        return IPAddressUtil.getIpsSet(getWhiteList());
    }

    public Set<String> getCustomDeniedUas() {
        Set<String> uas = new HashSet<String>();
        String uasString = (String) getSettingTypedValue("dos/drop-uas");
        if (StringUtils.isNotEmpty(uasString)) {
            String[] uasTokens = StringUtils.split(uasString, ',');
            for (String ua : uasTokens) {
                uas.add(StringUtils.trim(ua));
            }
        }
        return uas;
    }

    public Set<String> getDeniedSipUAs() {
        Set<String> uas = new HashSet<String>();
        addDeniedUserAgent(uas, "dos/friendly-scanner", "friendly-scanner");
        addDeniedUserAgent(uas, "dos/sipvicious", "sipvicious");
        addDeniedUserAgent(uas, "dos/sundayddr", "sundayddr");
        addDeniedUserAgent(uas, "dos/iwar", "iWar");
        addDeniedUserAgent(uas, "dos/sip-scan", "sip-scan");
        addDeniedUserAgent(uas, "dos/sipsak", "sipsak");
        uas.addAll(getCustomDeniedUas());
        return uas;
    }

    private void addDeniedUserAgent(Set<String> uas, String pathToSetting, String ua) {
        if ((Boolean) getSettingTypedValue(pathToSetting)) {
            uas.add(ua);
        }
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("firewall/firewall.xml");
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) FirewallManager.FEATURE);
    }

    public void setUnmanagedDefault(boolean unmanagedDefault) {
        m_unmanagedDefault = unmanagedDefault;
    }
}
