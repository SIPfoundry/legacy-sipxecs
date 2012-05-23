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
package org.sipfoundry.sipxconfig.time;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.List;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.apache.commons.net.util.SubnetUtils.SubnetInfo;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.YamlConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;

public class NtpConfig implements ConfigProvider {
    private NtpManager m_ntpManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(NtpManager.FEATURE)) {
            return;
        }

        Set<Location> locations = request.locations(manager);
        NtpSettings settings = m_ntpManager.getSettings();
        File gdir = manager.getGlobalDataDirectory();
        // check if marked as unmanaged service
        boolean unmanaged = settings.isServiceUnmanaged();
        ConfigUtils.enableCfengineClass(gdir, "ntpd_unmanaged.cfdat", unmanaged, "unmanaged_ntpd");

        // check if feature enabled, mark it as disable if unmanaged service
        boolean enabled = false;
        if (!unmanaged) {
            enabled = manager.getFeatureManager().isFeatureEnabled(NtpManager.FEATURE);
        }
        ConfigUtils.enableCfengineClass(gdir, "ntpd.cfdat", enabled, NtpManager.FEATURE.getId());

        if (enabled) {
            Boolean provideTime = settings.isProvideTimeSettingsEnabled();
            for (Location location : locations) {
                File dir = manager.getLocationDataDirectory(location);
                Writer ntpd = new FileWriter(new File(dir, "ntpd.yaml"));
                try {
                    writeNtpdConfig(ntpd, settings.isLocalClockEnabled(), settings.getNtpServers(), provideTime,
                            settings.getAlowedSubnetInfo(), settings.permitLoopBack(), settings.permitSync(),
                            settings.getDriftFile());
                } finally {
                    IOUtils.closeQuietly(ntpd);
                }
            }
        }
    }

    void writeNtpdConfig(Writer w, Boolean localClockEnabled, List<String> servers, Boolean provideTime,
            List<SubnetInfo> subnetInfo, Boolean loopback, Boolean sync, String driftFile) throws IOException {
        YamlConfiguration c = new YamlConfiguration(w);
        c.write("local_clock", localClockEnabled);
        c.write("loopback", loopback);
        c.write("sync", sync);
        c.write("drift_file", driftFile);
        c.startArray("ntp_servers");
        if (!servers.isEmpty()) {
            for (String server : servers) {
                c.nextElement();
                c.write(":address", server);
            }
        }
        c.endArray();
        c.startArray("subnets");
        if (provideTime) {
            if (!subnetInfo.isEmpty()) {
                for (SubnetInfo subnet : subnetInfo) {
                    c.nextElement();
                    c.write(":ip", subnet.getAddress());
                    c.write(":mask", subnet.getNetmask());
                }
            }
        }
        c.endArray();
    }

    public void setNtpManager(NtpManager ntpManager) {
        m_ntpManager = ntpManager;
    }
}
