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
package org.sipfoundry.sipxconfig.presence;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Collection;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.springframework.beans.factory.annotation.Required;

public class PresenceConfig implements ConfigProvider {
    private PresenceServer m_presenceServer;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(PresenceServer.FEATURE)) {
            return;
        }

        Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(
                PresenceServer.FEATURE);
        if (locations.isEmpty()) {
            return;
        }

        PresenceSettings settings = m_presenceServer.getSettings();
        Domain domain = manager.getDomainManager().getDomain();
        for (Location location : locations) {
            File locationDataDirectory = manager.getLocationDataDirectory(location);
            Writer wtr = new FileWriter(new File(locationDataDirectory, "presence-config.cfdat"));
            try {
                write(wtr, settings, location.getAddress(), domain);
            } finally {
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    void write(Writer wtr, PresenceSettings settings, String ipAddress, Domain domain) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.colonSeparated(wtr);
        config.writeSettings(settings.getSettings());
        config.write("SIP_PRESENCE_BIND_IP", ipAddress);
        config.write("SIP_PRESENCE_DOMAIN_NAME", domain.getName());
        config.write("SIP_PRESENCE_AUTHENTICATE_REALM", domain.getSipRealm());
    }

    @Required
    public void setPresenceServer(PresenceServer presenceServer) {
        m_presenceServer = presenceServer;
    }
}
