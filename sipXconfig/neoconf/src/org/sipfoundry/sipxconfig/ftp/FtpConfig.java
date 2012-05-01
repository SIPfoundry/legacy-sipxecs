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
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.sbc.SbcDevice;

public class FtpConfig implements ConfigProvider, DaoEventListener {
    private FtpManager m_ftpManager;
    private ConfigManager m_configManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(FtpManager.FTP_FEATURE, FtpManager.TFTP_FEATURE)) {
            return;
        }

        FtpSettings settings = m_ftpManager.getSettings();
        Set<Location> locations = request.locations(manager);
        for (Location l : locations) {
            File dir = manager.getLocationDataDirectory(l);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(FtpManager.FTP_FEATURE, l);
            ConfigUtils.enableCfengineClass(dir, "ftp.cfdat", enabled, "ftp");
            if (enabled) {
                Writer config = new FileWriter(new File(dir, "vsftp.config.part"));
                try {
                    writeConfig(config, settings);
                } finally {
                    IOUtils.closeQuietly(config);
                }
            }

            boolean tftpEnabled = manager.getFeatureManager().isFeatureEnabled(FtpManager.TFTP_FEATURE, l);
            ConfigUtils.enableCfengineClass(dir, "tftp.cfdat", tftpEnabled, "tftp");
        }
    }

    void writeConfig(Writer w, FtpSettings settings) throws IOException {
        KeyValueConfiguration c = KeyValueConfiguration.equalsSeparated(w);
        c.write(settings.getSettings().getSetting("vsftp-config"));
    }

    public void setFtpManager(FtpManager ftpManager) {
        m_ftpManager = ftpManager;
    }

    @Override
    public void onDelete(Object entity) {
        onChange(entity);
    }

    @Override
    public void onSave(Object entity) {
        onChange(entity);
    }

    public void onChange(Object entity) {
        // SBC address is used as default config for passive ftp connections
        if (entity instanceof SbcDevice) {
            m_configManager.configureEverywhere(FtpManager.FTP_FEATURE);
        }
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }
}
