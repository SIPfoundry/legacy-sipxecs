/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Collection;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.acd.Acd;
import org.sipfoundry.sipxconfig.acd.stats.AcdStats;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;

public class AcdHistoryConfig implements ConfigProvider {
    private AcdHistoricalStats m_historicalStats;
    private AddressManager m_addressManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(AcdStats.FEATURE, Acd.FEATURE)) {
            return;
        }

        Set<Location> locations = request.locations(manager);
        AcdHistoricalSettings settings = m_historicalStats.getSettings();
        Collection<Address> statsApis = m_addressManager.getAddresses(AcdStats.API_ADDRESS);
        for (Location location : locations) {
            if (!manager.getFeatureManager().isFeatureEnabled(AcdStats.FEATURE, location)) {
                continue;
            }
            File dir = manager.getLocationDataDirectory(location);
            File file = new File(dir, "sipxconfig-report-config.part");
            Writer wtr = new FileWriter(file);
            try {
                write(wtr, settings, statsApis);
            } finally {
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    void write(Writer wtr, AcdHistoricalSettings settings, Collection<Address> statsApis)
        throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.equalsSeparated(wtr);
        config.setValueFormat("\"%s\"");
        config.writeSettings(settings.getSettings());
        String statsUrls = StringUtils.join(statsApis, ';');
        config.write("CONFIG_SERVER_AGENT_URL", statsUrls);
    }

    public void setHistoricalStats(AcdHistoricalStats historicalStats) {
        m_historicalStats = historicalStats;
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }
}
