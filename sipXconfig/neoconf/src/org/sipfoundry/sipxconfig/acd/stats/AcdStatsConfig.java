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
package org.sipfoundry.sipxconfig.acd.stats;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.CfengineModuleConfiguration;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.springframework.beans.factory.annotation.Required;

public class AcdStatsConfig implements ConfigProvider {
    private AcdStats m_acdStats;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(AcdStats.FEATURE)) {
            return;
        }

        Set<Location> locations = request.locations(manager);
        AcdStatsSettings settings = m_acdStats.getSettings();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(AcdStats.FEATURE, location);
            ConfigUtils.enableCfengineClass(dir, "sipxacdstatistics.cfdat", enabled, "sipxacdstatistics");
            if (!enabled) {
                continue;
            }
            File file = new File(dir, "sipxacd-stats.cfdat");
            FileWriter wtr = new FileWriter(file);
            try {
                CfengineModuleConfiguration config = new CfengineModuleConfiguration(wtr);
                config.writeSettings(settings.getSettings());
            } finally {
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    @Required
    public void setAcdStats(AcdStats acdStats) {
        m_acdStats = acdStats;
    }
}
