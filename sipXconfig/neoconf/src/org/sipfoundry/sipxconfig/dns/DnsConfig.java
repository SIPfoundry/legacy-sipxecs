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
package org.sipfoundry.sipxconfig.dns;

import static java.lang.String.format;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.List;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.cfgmgt.CfengineModuleConfiguration;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.YamlConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.registrar.Registrar;

public class DnsConfig implements ConfigProvider {
    private DnsManager m_dnsManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(DnsManager.FEATURE, LocationsManager.FEATURE, ProxyManager.FEATURE, Registrar.FEATURE,
                ImManager.FEATURE)) {
            return;
        }

        File gdir = manager.getGlobalDataDirectory();
        DnsSettings settings = m_dnsManager.getSettings();
        // check if marked as unmanaged service
        boolean unmanaged = settings.isServiceUnmanaged();
        ConfigUtils.enableCfengineClass(gdir, "dns_unmanaged.cfdat", unmanaged, "unmanaged_dns");

        AddressManager am = manager.getAddressManager();
        String domain = manager.getDomainManager().getDomainName();
        List<Location> all = manager.getLocationManager().getLocationsList();
        Set<Location> locations = request.locations(manager);
        long serNo = System.currentTimeMillis();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            List<Address> dns = am.getAddresses(DnsManager.DNS_ADDRESS, location);

            // If there are no dns servers define, there is no reason to touch resolv.conf
            boolean resolvOn = dns.size() > 0 && !unmanaged;
            ConfigUtils.enableCfengineClass(dir, "resolv.cfdat", resolvOn, "resolv");
            if (resolvOn) {
                Writer resolv = new FileWriter(new File(dir, "resolv.conf.part"));
                try {
                    writeResolv(resolv, location, domain, dns);
                } finally {
                    IOUtils.closeQuietly(resolv);
                }
            }

            boolean namedOn = manager.getFeatureManager().isFeatureEnabled(DnsManager.FEATURE, location) && !unmanaged;
            ConfigUtils.enableCfengineClass(dir, "sipxdns.cfdat", namedOn, "sipxdns");
            if (!namedOn) {
                continue;
            }

            Writer dat = new FileWriter(new File(dir, "named.cfdat"));
            try {
                writeSettings(dat, settings);
            } finally {
                IOUtils.closeQuietly(dat);
            }

            List<Address> proxy = am.getAddresses(ProxyManager.TCP_ADDRESS, location);
            List<ResourceRecords> rrs = m_dnsManager.getResourceRecords(location);
            List<Address> im = am.getAddresses(ImManager.XMPP_ADDRESS, location);
            Writer zone = new FileWriter(new File(dir, "zone.yaml"));
            try {
                writeZoneConfig(zone, domain, all, proxy, im, dns, rrs, serNo);
            } finally {
                IOUtils.closeQuietly(zone);
            }
        }
    }

    void writeResolv(Writer w, Location l, String domain, List<Address> dns) throws IOException {
        // Only write out search if domain is not FQDN
        if (!l.getHostname().equals(domain)) {
            w.write(format("search %s\n", domain));
        }

        // write local dns server first if it exists
        StringBuilder nm = new StringBuilder();
        for (Address a : dns) {
            String line = format("nameserver %s\n", a.getAddress());
            if (l.getAddress().equals(a.getAddress())) {
                nm.insert(0, line);
            } else {
                nm.append(line);
            }
        }
        w.write(nm.toString());
    }

    void writeSettings(Writer w, DnsSettings settings) throws IOException {
        CfengineModuleConfiguration config = new CfengineModuleConfiguration(w);
        String fwders = "";
        List<Address> fwd = settings.getDnsForwarders();
        if (fwd != null && fwd.size() > 0) {
            fwders = StringUtils.join(settings.getDnsForwarders(), ';') + ';';
        }
        config.write("dnsForwarders", fwders);
    }

    void writeZoneConfig(Writer w, String domain, List<Location> all, List<Address> proxy,
            List<Address> im, List<Address> dns, List<ResourceRecords> rrs, long serNo) throws IOException {
        YamlConfiguration c = new YamlConfiguration(w);
        c.write("serialno", serNo);
        c.write("sip_protocols", "[ udp, tcp, tls ]");
        c.write("naptr_protocols", "[ udp, tcp ]");
        c.write("domain", domain);
        writeServerYaml(c, all, "proxy_servers", proxy);
        c.startArray("resource_records");
        if (rrs != null) {
            for (ResourceRecords rr : rrs) {
                c.nextElement();
                writeResourceRecords(c, all, rr);
            }
        }
        c.endArray();
        writeServerYaml(c, all, "dns_servers", dns);
        writeServerYaml(c, all, "im_servers", im);
        writeServerYaml(c, all, "all_servers", Location.toAddresses(DnsManager.DNS_ADDRESS, all));
    }

    /**
     * my-id : [ { :name: my-fqdn, :ipv4: 1.1.1.1 }, ... ]
     */
    void writeServerYaml(YamlConfiguration c, List<Location> all, String id, List<Address> addresses)
        throws IOException {
        c.startArray(id);
        if (addresses != null) {
            for (Address a : addresses) {
                c.nextElement();
                writeAddress(c, all, a.getAddress(), a.getPort());
            }
        }
        c.endArray();
    }

    void writeAddress(YamlConfiguration c, List<Location> all, String address, int port) throws IOException {
        c.write(":name", getHostname(all, address));
        c.write(":ipv4", address);
        c.write(":port", port);
    }

    void writeResourceRecords(YamlConfiguration c, List<Location> all, ResourceRecords rr) throws IOException {
        c.write(":proto", rr.getProto());
        c.write(":resource", rr.getResource());
        c.startArray(":records");
        for (DnsRecord r : rr.getRecords()) {
            c.nextElement();
            writeAddress(c, all, r.getAddress(), r.getPort());
        }
        c.endArray();
    }

    String getHostname(List<Location> locations, String ip) {
        for (Location l : locations) {
            if (ip.equals(l.getAddress())) {
                return l.getFqdn();
            }
        }
        throw new IllegalArgumentException("Cannot find hostname for IP address " + ip);
    }

    public void setDnsManager(DnsManager dnsManager) {
        m_dnsManager = dnsManager;
    }
}
