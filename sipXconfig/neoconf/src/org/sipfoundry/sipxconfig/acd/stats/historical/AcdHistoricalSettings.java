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
package org.sipfoundry.sipxconfig.acd.stats.historical;

import java.util.Collection;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.acd.stats.AcdStats;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class AcdHistoricalSettings extends PersistableSettings {
    private AddressManager m_addressManager;
    private FeatureManager m_featureManager;

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxacdreports/sipxacdreports.xml");
    }

    @SettingEntry(path = "stats-config/CONFIG_SERVER_REPORT")
    public boolean getReportEnabled() {
        return m_featureManager.isFeatureEnabled(AcdStats.FEATURE);
    }

    @SettingEntry(path = "stats-config/CONFIG_SERVER_AGENT_URL")
    public String getConfigAgentUrl() {
        Collection<Address> addresses = m_addressManager.getAddresses(AcdStats.API_ADDRESS);
        StringBuilder urls = new StringBuilder('\'');
        urls.append(StringUtils.join(addresses, ';'));
        urls.append('\'');
        return urls.toString();
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    @Override
    public String getBeanId() {
        return "acdHistoricalSettings";
    }
}
