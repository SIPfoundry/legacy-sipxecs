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

import org.sipfoundry.sipxconfig.acd.Acd;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;

public class AcdHistoryConfig implements ConfigProvider {
    private AcdHistoricalStats m_historicalStats;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (request.applies(AcdHistoricalStats.FEATURE, Acd.FEATURE)) {
            Location location = manager.getFeatureManager().getLocationForEnabledFeature(AcdHistoricalStats.FEATURE);
            if (location != null) {
                AcdHistoricalSettings settings = m_historicalStats.getSettings();
                File file = new File(manager.getLocationDataDirectory(location), "sipxacd-report.cfdat");
                FileWriter wtr = new FileWriter(file);
                KeyValueConfiguration config = new KeyValueConfiguration(wtr);
                config.write(settings.getSettings());
                config.write("LOCATION_FQDN", location.getFqdn());
            }
        }
    }

}
