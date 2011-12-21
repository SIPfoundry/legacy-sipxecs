/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd.stats.historical;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.acd.Acd;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;

public class AcdHistoryConfig implements ConfigProvider {
    private AcdHistoricalStats m_historicalStats;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(AcdHistoricalStats.FEATURE, Acd.FEATURE)) {
            return;
        }
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(
                AcdHistoricalStats.FEATURE);
        if (locations.isEmpty()) {
            return;
        }
        AcdHistoricalSettings settings = m_historicalStats.getSettings();
        for (Location location : locations) {
            File file = new File(manager.getLocationDataDirectory(location), "sipxacd-report.cfdat");
            Writer wtr = new FileWriter(file);
            try {
                write(wtr, settings, location);
            } finally {
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    void write(Writer wtr, AcdHistoricalSettings settings, Location location) throws IOException {
        KeyValueConfiguration config = new KeyValueConfiguration(wtr);
        config.write(settings.getSettings());
        config.write("LOCATION_FQDN", location.getFqdn());
    }
}
