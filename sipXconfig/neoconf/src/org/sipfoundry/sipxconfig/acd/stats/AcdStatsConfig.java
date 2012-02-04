/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
                config.write(settings.getSettings());
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
