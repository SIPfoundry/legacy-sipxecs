/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.cfgmgt.CfengineModuleConfiguration;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
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

        AddressManager am = manager.getAddressManager();
        List<Address> proxy = am.getAddresses(ProxyManager.TCP_ADDRESS, this);
        List<Address> reg = am.getAddresses(Registrar.TCP_ADDRESS, this);
        List<Address> im = am.getAddresses(ImManager.XMPP_ADDRESS, this);
        List<Address> dns = am.getAddresses(DnsManager.DNS_ADDRESS, this);
        String domain = manager.getDomainManager().getDomainName();
        List<Location> all = manager.getLocationManager().getLocationsList();
        DnsSettings settings = m_dnsManager.getSettings();
        Set<Location> locations = request.locations(manager);
        long serNo = System.currentTimeMillis();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);

            // If there are no dns servers define, there is no reason to touch resolv.conf
            boolean resolvOn = dns.size() > 0;
            ConfigUtils.enableCfengineClass(dir, "resolv.cfdat", resolvOn, "resolv");
            if (resolvOn) {
                Writer resolv = new FileWriter(new File(dir, "resolv.conf.part"));
                try {
                    writeResolv(resolv, location, domain, dns);
                } finally {
                    IOUtils.closeQuietly(resolv);
                }
            }

            boolean namedOn = manager.getFeatureManager().isFeatureEnabled(DnsManager.FEATURE, location);
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

            Writer zone = new FileWriter(new File(dir, "zone.yaml"));
            try {
                writeZoneConfig(zone, domain, all, proxy, reg, im, dns, serNo);
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

    void writeZoneConfig(Writer w, String domain, List<Location> all, List<Address> proxy, List<Address> reg,
            List<Address> im, List<Address> dns, long serNo) throws IOException {
        w.write(format("serialno: %d\n", serNo));
        w.write("sip_protocols: [ udp, tcp, tls ]\n");
        w.write("naptr_protocols: [ udp, tcp ]\n");
        w.write(format("domain: %s\n", domain));
        writeServerYaml(w, all, "proxy_servers", proxy);
        writeServerYaml(w, all, "registrar_servers", reg);
        writeServerYaml(w, all, "dns_servers", dns);
        writeServerYaml(w, all, "im_servers", im);
        writeServerYaml(w, all, "all_servers", toAddresses(all));
    }

    List<Address> toAddresses(List<Location> locations) {
        List<Address> addresses = new ArrayList<Address>(locations.size());
        for (Location l : locations) {
            addresses.add(new Address(l.getAddress()));
        }
        return addresses;
    }

    /**
     * my-id : [ { :name: my-fqdn, :ipv4: 1.1.1.1 }, ... ]
     */
    void writeServerYaml(Writer w, List<Location> locations, String id, List<Address> addresses) throws IOException {
        w.write(format("%s: [ ", id));
        int i = 0;
        if (addresses != null) {
            for (Address address : addresses) {
                if (i++ != 0) {
                    w.write(", ");
                }
                String name = getHostname(locations, address.getAddress());
                w.write(format("\n { :name: %s, :ipv4: %s }", name, address.getAddress()));
            }
        }
        w.write(" ]\n");
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
