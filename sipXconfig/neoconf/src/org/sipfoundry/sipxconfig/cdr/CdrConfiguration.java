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
package org.sipfoundry.sipxconfig.cdr;

import static org.apache.commons.lang.StringUtils.isEmpty;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.List;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
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

        Set<Location> locations = request.locations(manager);
        CdrSettings settings = m_cdrManager.getSettings();
        List<Location> proxyLocations = manager.getFeatureManager().getLocationsForEnabledFeature(ProxyManager.FEATURE);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            String datfile = "sipxcdr.cfdat";
            if (!location.isPrimary()) {
                ConfigUtils.enableCfengineClass(dir, datfile, false, CdrManager.FEATURE.getId());
                continue;
            }
            ConfigUtils.enableCfengineClass(dir, datfile, true, CdrManager.FEATURE.getId(), "postgres");
            FileWriter wtr = new FileWriter(new File(dir, "callresolver-config"));
            try {
                write(wtr, proxyLocations, settings);
            } finally {
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    static String cseHosts(List<Location> locations, int port) {
        StringBuilder cseHosts = new StringBuilder();
        boolean first = true;
        for (Location location : locations) {
            if (first) {
                cseHosts.append(location.getFqdn()).append(':').append(port);
                first = false;
                continue;
            }
            cseHosts.append(", ");
            cseHosts.append(location.getFqdn()).append(':').append(port);
        }
        return cseHosts.toString();
    }

    void write(Writer wtr, List<Location> proxyLocations, CdrSettings settings) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.colonSeparated(wtr);
        config.writeSettings(settings.getSettings());
        // legacy, not sure if it should be just ip or home interface
        config.write("SIP_CALLRESOLVER_AGENT_ADDR", "0.0.0.0");
        String cseHosts = cseHosts(proxyLocations, AdminContext.SIPXCDR_DB_ADDRESS.getCanonicalPort());
        if (!isEmpty(cseHosts)) {
            config.write("SIP_CALLRESOLVER_CSE_HOSTS", cseHosts);
        }
    }

    public void setCdrManager(CdrManager cdrManager) {
        m_cdrManager = cdrManager;
    }
}
