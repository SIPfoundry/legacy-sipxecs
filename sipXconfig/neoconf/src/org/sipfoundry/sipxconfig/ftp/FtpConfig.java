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
package org.sipfoundry.sipxconfig.ftp;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.sbc.SbcManager;

public class FtpConfig implements ConfigProvider {
    private FtpManager m_ftpManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(FtpManager.FTP_FEATURE, SbcManager.FEATURE)) {
            return;
        }

        FtpSettings settings = m_ftpManager.getSettings();
        Set<Location> locations = request.locations(manager);
        for (Location l : locations) {
            File dir = manager.getLocationDataDirectory(l);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(FtpManager.FTP_FEATURE, l);
            ConfigUtils.enableCfengineClass(dir, "ftp.cfdat", enabled, "ftp");
            if (!enabled) {
                continue;
            }
            Writer config = new FileWriter(new File(dir, "vsftp.config.part"));
            try {
                writeConfig(config, settings);
            } finally {
                IOUtils.closeQuietly(config);
            }
        }
    }

    void writeConfig(Writer w, FtpSettings settings) throws IOException {
        KeyValueConfiguration c = KeyValueConfiguration.equalsSeparated(w);
        c.write(settings.getSettings().getSetting("vsftp-config"));
    }

    public void setFtpManager(FtpManager ftpManager) {
        m_ftpManager = ftpManager;
    }
}
