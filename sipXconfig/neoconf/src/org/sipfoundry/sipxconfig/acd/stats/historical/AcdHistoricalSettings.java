/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd.stats.historical;

import java.util.Collection;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.acd.stats.AcdStats;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class AcdHistoricalSettings extends BeanWithSettings {
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
}
