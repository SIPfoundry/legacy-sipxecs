/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cdr;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.List;

import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;

public class CdrConfiguration implements ConfigProvider {
    private CdrManager m_cdrManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (request.applies(CdrManager.FEATURE, ProxyManager.FEATURE)) {
            List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(ProxyManager.FEATURE);
            CdrSettings settings = m_cdrManager.getSettings();
            StringBuilder cseHosts = new StringBuilder();
            for (int i = 0; i < locations.size(); i++) {
                if (i > 0) {
                    cseHosts.append(',');
                }
                cseHosts.append(locations.get(i).getAddress()).append(':').append(settings.getAgentPort());
            }

            for (Location location : locations) {
                File dir = manager.getLocationDataDirectory(location);
                FileWriter wtr = new FileWriter(new File(dir, "callresolver-config"));
                KeyValueConfiguration config = new KeyValueConfiguration(wtr);
                config.write(settings.getSettings());
                // legacy, not sure if it should be just ip or home interface
                config.write("SIP_CALLRESOLVER_AGENT_ADDR", "0.0.0.0");
                config.write("SIP_CALLRESOLVER_CSE_HOSTS", cseHosts.toString());
            }
        }
    }
}
