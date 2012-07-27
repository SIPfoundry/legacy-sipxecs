/**
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
package org.sipfoundry.sipxconfig.admin;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;

public class AdminConfig implements ConfigProvider {
    private AdminContext m_adminContext;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(AdminContext.FEATURE)) {
            return;
        }

        Set<Location> locations = request.locations(manager);
        for (Location l : locations) {
            if (!l.isPrimary()) {
                continue;
            }
            File dir = manager.getLocationDataDirectory(l);
            Writer w = new FileWriter(new File(dir, "sipxconfig.properties.ui"));
            try {
                AdminSettings settings = m_adminContext.getSettings();
                writeConfig(w, settings);
            } finally {
                IOUtils.closeQuietly(w);
            }
        }
    }

    void writeConfig(Writer w, AdminSettings settings) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.equalsSeparated(w);
        config.writeSettings(settings.getSettings().getSetting("configserver-config"));
    }

    public void setAdminContext(AdminContext adminContext) {
        m_adminContext = adminContext;
    }
}
