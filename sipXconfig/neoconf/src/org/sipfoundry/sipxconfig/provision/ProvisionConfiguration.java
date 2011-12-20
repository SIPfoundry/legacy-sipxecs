/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.provision;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;

public class ProvisionConfiguration implements ConfigProvider {
    private Provision m_provision;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(Provision.FEATURE)) {
            return;
        }

        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(Provision.FEATURE);
        ProvisionSettings settings = m_provision.getSettings();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            Writer prov = new FileWriter(new File(dir, "sipxprovision-config.cfdat"));
            try {
                write(prov, settings);
            } finally {
                IOUtils.closeQuietly(prov);
            }
        }
    }

    void write(Writer wtr, ProvisionSettings settings) throws IOException {
        KeyValueConfiguration config = new KeyValueConfiguration(wtr, "=");
        config.write(settings.getSettings().getSetting("provision-config"));
    }
}
