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
package org.sipfoundry.sipxconfig.dhcp;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Collection;
import java.util.List;
import java.util.Set;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.YamlConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dns.DnsManager;
import org.sipfoundry.sipxconfig.ftp.FtpManager;
import org.sipfoundry.sipxconfig.time.NtpManager;

public class DhcpConfig implements ConfigProvider {
    private DhcpManager m_dhcpManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(DhcpManager.FEATURE)) {
            return;
        }

        AddressManager addressManager = manager.getAddressManager();
        Address tftp = addressManager.getSingleAddress(FtpManager.TFTP_ADDRESS);
        Address admin = addressManager.getSingleAddress(AdminContext.HTTP_ADDRESS);
        List<Address> dns = addressManager.getAddresses(DnsManager.DNS_ADDRESS);
        List<Address> ntp = addressManager.getAddresses(NtpManager.NTP_SERVER);

        File gdir = manager.getGlobalDataDirectory();
        DhcpSettings settings  = m_dhcpManager.getSettings();
        boolean unmanaged = settings.isServiceUnmanaged();
        ConfigUtils.enableCfengineClass(gdir, "dhcpd_unmanaged.cfdat", unmanaged, "unmanaged_dhcpd");

        Set<Location> locations = request.locations(manager);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(DhcpManager.FEATURE, location) && !unmanaged;
            ConfigUtils.enableCfengineClass(dir, "dhcpd.cfdat", enabled, "dhcpd");
            if (!enabled) {
                continue;
            }
            Writer w = new FileWriter(new File(dir, "dhcpd.yaml"));
            try {
                writeConfig(w, settings, tftp, admin, dns, ntp);
            } finally {
                IOUtils.closeQuietly(w);
            }
        }
    }

    protected void writeConfig(Writer w, DhcpSettings settings, Address tftp, Address admin,
        Collection<Address> dns, Collection<Address> ntp) throws IOException {
        YamlConfiguration c = new YamlConfiguration(w);
        c.writeSettings(settings.getSettings().getSetting("dhcpd-config"));
        c.write("tftp", tftp != null ? tftp.getAddress() : "");
        c.write("config", admin.stripProtocol());
        c.writeInlineArray("ntp", CollectionUtils.collect(ntp, Address.GET_IP));
        c.writeInlineArray("dns", CollectionUtils.collect(dns, Address.GET_IP));
    }

    public void setDhcpManager(DhcpManager dhcpManager) {
        m_dhcpManager = dhcpManager;
    }
}
