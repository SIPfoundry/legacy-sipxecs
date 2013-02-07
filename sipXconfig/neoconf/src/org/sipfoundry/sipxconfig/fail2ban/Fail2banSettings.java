/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.fail2ban;

import java.util.Arrays;
import java.util.Collection;

import org.sipfoundry.commons.util.IPAddressUtil;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class Fail2banSettings extends PersistableSettings implements DeployConfigOnEdit {
    private static final String IGNORE_IP = "config/ignoreip";
    private FirewallManager m_firewallManager;

    @Override
    public String getBeanId() {
        return "fail2banSettings";
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("fail2ban/fail2ban.xml");
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Arrays.asList((Feature) FirewallManager.FEATURE, (Feature) Fail2banManager.FEATURE);
    }

    @Override
    public void initialize() {
        addDefaultBeanSettingHandler(new Defaults());
    }

    public void validate() {
        String ipList = (String) getSettingTypedValue(IGNORE_IP);
        if (!IPAddressUtil.validateIpList(ipList)) {
            throw new UserException("&msg.invalidcidr", ipList);
        }
    }

    public boolean isServiceUnmanaged() {
        return (Boolean) getSettingTypedValue("config/unmanaged");
    }

    public void setFirewallManager(FirewallManager manager) {
        m_firewallManager = manager;
    }

    public class Defaults {

        @SettingEntry(path = IGNORE_IP)
        public String getIgnoreIp() {
            return m_firewallManager.getSettings().getWhiteList();
        }

        @SettingEntry(paths = {
                "rules/dos/ignoreip", "rules/invite/ignoreip", "rules/register/ignoreip", "rules/options/ignoreip",
                "rules/ack/ignoreip", "rules/subscribe/ignoreip"
                })
        public String getSipRuleIgnoreIp() {
            return getSettingValue(IGNORE_IP);
        }
    }
}
