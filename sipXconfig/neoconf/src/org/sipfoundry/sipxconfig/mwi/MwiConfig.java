/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mwi;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Collection;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.annotation.Required;

public class MwiConfig implements ConfigProvider {
    private Mwi m_mwi;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (request.applies(Mwi.FEATURE)) {
            Location[] allLocations = manager.getLocationManager().getLocations();
            StringBuilder validIps = new StringBuilder(allLocations[0].getAddress());
            for (int i = 1; i < validIps.length(); i++) {
                validIps.append(",").append(allLocations[i].getAddress());
            }
            Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(Mwi.FEATURE);
            for (Location location : locations) {
                MwiSettings settings = m_mwi.getSettings();
                File file = new File(manager.getLocationDataDirectory(location), "status-config.cfdat");
                FileWriter wtr = new FileWriter(file);
                KeyValueConfiguration config = new KeyValueConfiguration(wtr);
                config.write(settings.getSettings());
                DomainManager dm = manager.getDomainManager();
                config.write("SIP_STATUS_BIND_IP", location.getAddress());
                config.write("SIP_STATUS_AUTHENTICATE_REALM", dm.getAuthorizationRealm());
                config.write("SIP_STATUS_DOMAIN_NAME", dm.getDomainName());
                config.write("SIP_STATUS_HTTP_VALID_IPS", validIps.toString());
            }
        }
    }

    @Required
    public void setMwi(Mwi mwi) {
        m_mwi = mwi;
    }
}
