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
import java.io.Writer;
import java.util.List;

import org.apache.commons.io.IOUtils;
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
        if (!request.applies(CdrManager.FEATURE, ProxyManager.FEATURE)) {
            return;
        }

        List<Location> proxyLocations = manager.getFeatureManager().getLocationsForEnabledFeature(ProxyManager.FEATURE);
        if (proxyLocations.isEmpty()) {
            return;
        }

        CdrSettings settings = m_cdrManager.getSettings();
        for (Location location : proxyLocations) {
            File dir = manager.getLocationDataDirectory(location);
            FileWriter wtr = new FileWriter(new File(dir, "callresolver-config"));
            try {
                write(wtr, proxyLocations, settings);
            } finally {
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    static String cseHosts(List<Location> locations, int port) {
        StringBuilder cseHosts = new StringBuilder("localhost");
        for (int i = 0; i < locations.size(); i++) {
            cseHosts.append(", ");
            cseHosts.append(locations.get(i).getFqdn()).append(':').append(port);
        }
        return cseHosts.toString();
    }

    void write(Writer wtr, List<Location> proxyLocations, CdrSettings settings) throws IOException {
        KeyValueConfiguration config = new KeyValueConfiguration(wtr);
        config.write(settings.getSettings());
        // legacy, not sure if it should be just ip or home interface
        config.write("SIP_CALLRESOLVER_AGENT_ADDR", "0.0.0.0");
        String cseHosts = cseHosts(proxyLocations, settings.getPostresPort());
        config.write("SIP_CALLRESOLVER_CSE_HOSTS", cseHosts);
    }

    public void setCdrManager(CdrManager cdrManager) {
        m_cdrManager = cdrManager;
    }
}
