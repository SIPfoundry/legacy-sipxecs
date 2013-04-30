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
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.cfgmgt.CfengineModuleConfiguration;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.YamlConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.registrar.Registrar;

public class DnsConfig implements ConfigProvider {
    private static final Log LOG = LogFactory.getLog(DnsConfig.class);
    private DnsManager m_dnsManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(DnsManager.FEATURE, LocationsManager.FEATURE, ProxyManager.FEATURE, Registrar.FEATURE,
                ImManager.FEATURE)) {
            return;
        }

        DnsSettings settings = m_dnsManager.getSettings();
        AddressManager am = manager.getAddressManager();
        String domain = manager.getDomainManager().getDomainName();
        String networkDomain = Domain.getDomain().getNetworkName();
        List<Location> all = manager.getLocationManager().getLocationsList();
        Set<Location> locations = request.locations(manager);

        // 32 bit unsigned runs out in year 2148 which is 136 yrs past july 16, 2012
        long serNo = (System.currentTimeMillis() / 1000) - 1342487870;

        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            List<Address> dns = am.getAddresses(DnsManager.DNS_ADDRESS, location);

            // If there are no dns servers define, there is no reason to touch resolv.conf
            boolean resolvOn = dns.size() > 0;
            if (resolvOn) {
                Writer resolv = new FileWriter(new File(dir, "resolv.conf.part"));
                try {
                    writeResolv(resolv, location, networkDomain, dns);
                } finally {
                    IOUtils.closeQuietly(resolv);
                }
            }

            boolean namedOn = manager.getFeatureManager().isFeatureEnabled(DnsManager.FEATURE, location);
            Writer dat = new FileWriter(new File(dir, "named.cfdat"));
            try {
                writeSettings(dat, namedOn, resolvOn, settings);
            } finally {
                IOUtils.closeQuietly(dat);
            }

            if (!namedOn) {
                continue;
            }

            List<Address> proxy = am.getAddresses(ProxyManager.TCP_ADDRESS, location);
            List<ResourceRecords> rrs = m_dnsManager.getResourceRecords(location);
            List<Address> im = am.getAddresses(ImManager.XMPP_ADDRESS, location);
            Writer zone = new FileWriter(new File(dir, "zone.yaml"));
            try {
                boolean generateARecords = domain.equals(networkDomain);
                writeZoneConfig(zone, domain, all, proxy, im, dns, rrs, serNo, generateARecords);
            } finally {
                IOUtils.closeQuietly(zone);
            }
        }
    }

    void writeResolv(Writer w, Location l, String domain, List<Address> dns) throws IOException {
        w.write(format("search %s\n", domain));

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

    void writeSettings(Writer w, boolean namedOn, boolean resolvOn, DnsSettings settings) throws IOException {
        CfengineModuleConfiguration config = new CfengineModuleConfiguration(w);
        config.writeClass("resolv", resolvOn);
        config.writeClass("sipxdns", namedOn);
        String fwders = "";
        List<Address> fwd = settings.getDnsForwarders();
        if (fwd != null && fwd.size() > 0) {
            fwders = StringUtils.join(fwd, ';') + ';';
        }
        config.write("sipxdns_forwarders", fwders);
        config.writeSettings("sipxdns_", settings.getSettings());
    }

    void writeZoneConfig(Writer w, String domain, List<Location> all, List<Address> proxy,
        List<Address> im, List<Address> dns, List<ResourceRecords> rrs, long serNo, boolean generateARecords)
        throws IOException {
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
        List<Address> dnsAddresses = new ArrayList<Address>();
        if (generateARecords) {
            dnsAddresses = Location.toAddresses(DnsManager.DNS_ADDRESS, all);
        }
        writeServerYaml(c, all, "all_servers", dnsAddresses);
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
                writeAddress(c, all, a.getAddress(), a.getPort(), false);
            }
        }
        c.endArray();
    }

    void writeAddress(YamlConfiguration c, List<Location> all, String address, int port, boolean rewrite)
        throws IOException {
        String host = getHostname(all, address, rewrite);
        if (host != null) {
            c.write(":name", host);
            c.write(":ipv4", address);
            c.write(":port", port);
            if (rewrite) {
                c.write(":target", getHostname(all, address, false));
            }
        }
    }

    void writeResourceRecords(YamlConfiguration c, List<Location> all, ResourceRecords rr) throws IOException {
        c.write(":proto", rr.getProto());
        c.write(":resource", rr.getResource());
        c.startArray(":records");
        for (DnsRecord r : rr.getRecords()) {
            c.nextElement();
            writeAddress(c, all, r.getAddress(), r.getPort(), true);
        }
        c.endArray();
    }

    String getHostname(List<Location> locations, String ip, boolean rewrite) {
        for (Location l : locations) {
            if (ip.equals(l.getAddress())) {
                if (rewrite) {
                    return l.getHostnameInSipDomain();
                }
                return l.getFqdn();
            }
        }
        LOG.warn("No hostname found for " + ip + ", could be unmanged service");
        return null;
    }

    public void setDnsManager(DnsManager dnsManager) {
        m_dnsManager = dnsManager;
    }
}
