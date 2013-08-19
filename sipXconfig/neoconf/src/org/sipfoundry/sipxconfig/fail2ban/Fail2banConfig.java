/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.fail2ban;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.YamlConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.beans.factory.annotation.Required;

public class Fail2banConfig implements ConfigProvider {
    private static final String PREFIX = ":";
    private Fail2banManager m_fail2banManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(Fail2banManager.FEATURE)) {
            return;
        }
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(Fail2banManager.FEATURE);
        File gdir = manager.getGlobalDataDirectory();
        Fail2banSettings settings = m_fail2banManager.getSettings();
        boolean unmanaged = settings.isServiceUnmanaged();
        ConfigUtils.enableCfengineClass(gdir, "security_unmanaged.cfdat", unmanaged, "security_unmanaged");

        for (Location location : request.locations(manager)) {
            File dir = manager.getLocationDataDirectory(location);
            ConfigUtils.enableCfengineClass(dir, "security.cfdat", enabled, "security");

            Writer config = new FileWriter(new File(dir, "security.yaml"));
            try {
                writeConfig(config, settings, enabled);
            } finally {
                IOUtils.closeQuietly(config);
            }
        }
    }

    void writeConfig(Writer config, Fail2banSettings settings, boolean enabled) throws IOException {
        YamlConfiguration c = new YamlConfiguration(config);
        for (Setting setting : settings.getSettings().getSetting("config").getValues()) {
            writeSettings(c, setting, false);
        }
        c.startArray("siprules");
        if (enabled) {
            for (Setting setting : settings.getSettings().getSetting("rules").getValues()) {
                c.nextElement();
                c.write(":name", setting.getName());
                for (Setting configSetting : setting.getValues()) {
                    writeSettings(c, configSetting, true);
                }
            }
        }
        c.endArray();
    }

    private void writeSettings(YamlConfiguration c, Setting setting, boolean prefixed) throws IOException {
        String configname = setting.getName();
        if (configname.equalsIgnoreCase("ignoreip")) {
            String ipsValue = StringUtils.deleteWhitespace((String) setting.getTypedValue());
            List<String> ips = new ArrayList<String>();
            if (StringUtils.isNotEmpty(ipsValue)) {
                ips = Arrays.asList(StringUtils.split(ipsValue, ","));
                if (prefixed) {
                    configname = PREFIX + configname;
                }
            }
            c.writeArray(configname, ips);
        } else {
            if (prefixed) {
                configname = PREFIX + configname;
            }
            c.write(configname, setting.getTypedValue());
        }
    }

    @Required
    public void setFail2banManager(Fail2banManager manager) {
        m_fail2banManager = manager;
    }
}
