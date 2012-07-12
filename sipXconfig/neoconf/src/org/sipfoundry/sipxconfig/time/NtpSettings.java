/**
 *
 *
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
package org.sipfoundry.sipxconfig.time;


import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.net.util.SubnetUtils;
import org.apache.commons.net.util.SubnetUtils.SubnetInfo;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class NtpSettings extends PersistableSettings implements DeployConfigOnEdit {
    public static final String TIMEZONE_SETTING = "ntp-sysconfig/timezone";
    private static final String NTP_SERVER_0 = "ntp-config/server-0";
    private static final String NTP_SERVER_1 = "ntp-config/server-1";
    private static final String NTP_SERVER_2 = "ntp-config/server-2";
    private static final String NTP_SERVER_3 = "ntp-config/server-3";
    private static final String NTP_UNMANAGED_SERVER_0 = "ntp-unmanaged/server-0";
    private static final String NTP_UNMANAGED_SERVER_1 = "ntp-unmanaged/server-1";
    private static final String NTP_UNMANAGED_SERVER_2 = "ntp-unmanaged/server-2";
    private static final String NTP_UNMANAGED_SERVER_3 = "ntp-unmanaged/server-3";
    private static final String LOCAL_CLOCK_SETTING = "ntp-config/local-clock";
    private static final String PROVIDE_TIME_SETTING = "ntp-config/provide-time";
    private static final String ALLOWED_NETWORKS_SETTING = "ntp-config/networks-allowed";
    private static final String PERMIT_LOOPBACK_SETTING = "ntp-config/permit-loopback";
    private static final String PERMIT_SYNC_SETTING = "ntp-config/permit-sync";
    private static final String DRIFT_FILE_SETTING = "ntp-config/drift-file";
    private static final String UNMANAGED_SETTING = "ntp-unmanaged/unmanaged";

    @Override
    public String getBeanId() {
        return "ntpSettings";
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("ntp/ntp.xml");
    }

    public boolean isServiceUnmanaged() {
        return (Boolean) getSettingTypedValue(UNMANAGED_SETTING);
    }

    public boolean isLocalClockEnabled() {
        return (Boolean) getSettingTypedValue(LOCAL_CLOCK_SETTING);
    }

    public boolean isProvideTimeSettingsEnabled() {
        return (Boolean) getSettingTypedValue(PROVIDE_TIME_SETTING);
    }

    public boolean permitLoopBack() {
        return (Boolean) getSettingTypedValue(PERMIT_LOOPBACK_SETTING);
    }

    public boolean permitSync() {
        return (Boolean) getSettingTypedValue(PERMIT_SYNC_SETTING);
    }

    public String getDriftFile() {
        return (String) getSettingTypedValue(DRIFT_FILE_SETTING);
    }

    public String getTimezone() {
        return (String) getSettingValue(TIMEZONE_SETTING);
    }

    public List<SubnetInfo> getAlowedSubnetInfo() {
        String allowedNetworks = (String) getSettingTypedValue(ALLOWED_NETWORKS_SETTING);
        List<SubnetInfo> subnetInfo = new ArrayList<SubnetInfo>();
        if (isProvideTimeSettingsEnabled()) {
            String[] networks = StringUtils.split(allowedNetworks, ",");
            if (networks != null) {
                for (String cidNetwork : networks) {
                    try {
                        subnetInfo.add(new SubnetUtils(StringUtils.deleteWhitespace(cidNetwork)).getInfo());
                    } catch (IllegalArgumentException ex) {
                        continue;
                    }
                }
            }
        }
        return subnetInfo;
    }

    public List<String> getNtpServers() {
        List<String> servers = new ArrayList<String>();
        if (isServiceUnmanaged()) {
            addNtpServer(NTP_UNMANAGED_SERVER_0, servers);
            addNtpServer(NTP_UNMANAGED_SERVER_1, servers);
            addNtpServer(NTP_UNMANAGED_SERVER_2, servers);
            addNtpServer(NTP_UNMANAGED_SERVER_3, servers);
        } else {
            addNtpServer(NTP_SERVER_0, servers);
            addNtpServer(NTP_SERVER_1, servers);
            addNtpServer(NTP_SERVER_2, servers);
            addNtpServer(NTP_SERVER_3, servers);
        }
        return servers;
    }

    private void addNtpServer(String setting, List<String> servers) {
        String server = getSettingValue(setting);
        if (StringUtils.isNotBlank(server)) {
            servers.add(server);
        }
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) NtpManager.FEATURE);
    }
}
