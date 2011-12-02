/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.presence;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Collection;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.springframework.beans.factory.annotation.Required;

public class PresenceConfig implements ConfigProvider {
    private FeatureManager m_featureManager;
    private PresenceServer m_presenceServer;

    @Override
    public void replicate(ConfigManager manager) throws IOException {
        Collection<Location> locations = m_featureManager.getLocationsForEnabledFeature(PresenceServer.FEATURE);
        PresenceSettings settings = m_presenceServer.getSettings();
        for (Location location : locations) {
            File locationDataDirectory = manager.getLocationDataDirectory(location);
            FileWriter wtr = null;
            try {
                wtr = new FileWriter(new File(locationDataDirectory, "sipxpresence-config.cfdat"));
                KeyValueConfiguration config = new KeyValueConfiguration(wtr);
                config.write(settings.getSettings());
                config.write("SIP_PRESENCE_BIND_IP", location.getAddress());
            } finally {
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    @Required
    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    @Required
    public void setPresenceServer(PresenceServer presenceServer) {
        m_presenceServer = presenceServer;
    }
}
