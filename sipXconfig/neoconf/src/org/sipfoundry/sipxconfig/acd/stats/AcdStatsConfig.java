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
import java.util.List;

import org.sipfoundry.sipxconfig.acd.Acd;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.springframework.beans.factory.annotation.Required;

public class AcdStatsConfig implements ConfigProvider {
    private AcdStats m_acdStats;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(Acd.FEATURE)) {
            return;
        }

        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(AcdStats.FEATURE);
        for (Location location : locations) {
            AcdStatsSettings settings = m_acdStats.getSettings();
            File file = new File(manager.getLocationDataDirectory(location), "sipxacd-stats.cfdat");
            FileWriter wtr = new FileWriter(file);
            KeyValueConfiguration config = new KeyValueConfiguration(wtr, "=");
            config.write(settings.getSettings());
        }
    }

    @Required
    public void setAcdStats(AcdStats acdStats) {
        m_acdStats = acdStats;
    }
}
