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
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
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
        PresenceSettings settings = m_presenceServer.getSettings();
        String domainName = manager.getDomainManager().getDomainName();
        String realm = manager.getDomainManager().getAuthorizationRealm();
        for (Location location : locations) {
            File locationDataDirectory = manager.getLocationDataDirectory(location);
            FileWriter wtr = new FileWriter(new File(locationDataDirectory, "presence-config.cfdat"));
            try {
                KeyValueConfiguration config = new KeyValueConfiguration(wtr);
                config.write(settings.getSettings());
                config.write("SIP_PRESENCE_BIND_IP", location.getAddress());
                config.write("SIP_PRESENCE_DOMAIN_NAME", domainName);
                config.write("SIP_PRESENCE_AUTHENTICATE_REALM", realm);
            } finally {
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    @Required
    public void setPresenceServer(PresenceServer presenceServer) {
        m_presenceServer = presenceServer;
    }
}
