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
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Set;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Transformer;
import org.apache.commons.io.IOUtils;
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
import org.sipfoundry.sipxconfig.region.Region;
import org.sipfoundry.sipxconfig.region.RegionManager;
import org.sipfoundry.sipxconfig.registrar.Registrar;
import org.springframework.beans.factory.annotation.Required;

public class DnsConfig implements ConfigProvider {
    private static final String YML_PORT = ":port";
    private static final String YML_NAME = ":name";
    private static final String YML_DOMAIN = "domain";
    private static final String YML_TARGET = ":target";
    private static final String VIEW_NAME = "%s.view";
    private static final Log LOG = LogFactory.getLog(DnsConfig.class);
    private DnsManager m_dnsManager;
    private RegionManager m_regionManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(DnsManager.FEATURE, LocationsManager.FEATURE, ProxyManager.FEATURE, Registrar.FEATURE,
                ImManager.FEATURE, RegionManager.FEATURE_ID)) {
            return;
        }

        DnsSettings settings = m_dnsManager.getSettings();
        AddressManager am = manager.getAddressManager();
        String domain = manager.getDomainManager().getDomainName();
        String networkDomain = Domain.getDomain().getNetworkName();
        List<Location> all = manager.getLocationManager().getLocationsList();
        Set<Location> locations = request.locations(manager);
        List<Address> dns = am.getAddresses(DnsManager.DNS_ADDRESS, null);

        // 32 bit unsigned runs out in year 2148 which is 136 yrs past july 16, 2012
        long serNo = (System.currentTimeMillis() / 1000) - 1342487870;
        Collection<Region> regions = getRegionsInUse(all);

        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);

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
                writeSettings(dat, namedOn, resolvOn, regions, settings);
            } finally {
                IOUtils.closeQuietly(dat);
            }

            if (!namedOn) {
                continue;
            }
        }

        File gdir = manager.getGlobalDataDirectory();
        Writer named = new FileWriter(new File(gdir, "named.yaml"));
        try {
            writeNamedConfig(named, Domain.getDomain(), regions, settings.getDnsForwarders());
        } finally {
            IOUtils.closeQuietly(named);
        }

        boolean generateARecords = domain.equals(networkDomain) || domain.equals(all.get(0).getFqdn());
        for (Region region : regions) {
            Collection<DnsSrvRecord> records = m_dnsManager.getResourceRecords(region);
            String file = format(VIEW_NAME + ".yaml", region.getConfigFriendlyName());
            Writer zone = new FileWriter(new File(gdir, file));
            try {
                writeZoneConfig(zone, domain, all, dns, records, serNo, generateARecords);
            } finally {
                IOUtils.closeQuietly(zone);
            }
        }
    }

    List<Region> getRegionsInUse(Collection<Location> locations) throws IOException {
        List<Region> regions = new ArrayList<Region>();
        regions.add(Region.DEFAULT);
        // while the default region can never be selection by a location, it's always
        // in use
        for (Region r : m_regionManager.getRegions()) {
            // get a list of regions that are actually used by one or more locations
            for (Location location : locations) {
                if (r.getId().equals(location.getRegionId())) {
                    regions.add(r);
                    break;
                }
            }
        }
        Collections.sort(regions, new Comparator<Region>() {
            @Override
            public int compare(Region o1, Region o2) {
                // We need default region last. Eventually use will want to control the order
                // of the views.
                return o2.getId() - o1.getId();
            }
        });
        return regions;
    }

    void writeNamedConfig(Writer w, Domain d, Collection<Region> regions, Collection<Address> forwarders)
        throws IOException {
        YamlConfiguration c = new YamlConfiguration(w);
        Collection< ? > forwarderIps = CollectionUtils.collect(forwarders, Address.GET_IP);
        c.write(YML_DOMAIN, d.getName());
        c.writeArray("forwarders", forwarderIps);
        c.startArray("views");
        for (Region region : regions) {
            c.write(YML_NAME, format(VIEW_NAME, region.getConfigFriendlyName()));
            c.writeArray(":match_clients", safeAsList(region.getAddresses()));
            c.nextElement();
        }
        c.endArray();
    }

    final <T> List<T> safeAsList(T...a) {
        if (a == null) {
            return null;
        }
        return Arrays.asList(a);
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

    void writeSettings(Writer w, boolean namedOn, boolean resolvOn, Collection<Region> regions, DnsSettings settings)
        throws IOException {
        CfengineModuleConfiguration config = new CfengineModuleConfiguration(w);
        config.writeClass("resolv", resolvOn);
        config.writeClass("sipxdns", namedOn);
        @SuppressWarnings("unchecked")
        Collection<String> zones = CollectionUtils.collect(regions, new Transformer() {
            @Override
            public Object transform(Object arg0) {
                return format(VIEW_NAME, ((Region) arg0).getConfigFriendlyName());
            }
        });
        config.writeList("dnsviews", zones);
        config.writeSettings("sipxdns_", settings.getSettings());
    }

    void writeZoneConfig(Writer w, String domain, List<Location> all, List<Address> dns,
            Collection<DnsSrvRecord> rrs, long serNo, boolean generateARecords) throws IOException {
        YamlConfiguration c = new YamlConfiguration(w);
        c.write("serialno", serNo);
        c.write("naptr_protocols", "[ udp, tcp ]");
        c.write(YML_DOMAIN, domain);
        c.startArray("resource_records");
        String qualifiedTarget = null;
        boolean domainIsFqdn = domain.equals(all.get(0).getFqdn());
        if (domainIsFqdn) {
            qualifiedTarget = domain + ".";
        }
        if (rrs != null) {
            for (DnsSrvRecord rr : rrs) {
                c.nextElement();
                writeSrvRecord(c, rr, qualifiedTarget);
            }
        }
        c.endArray();
        writeServerYaml(c, all, "dns_servers", dns, qualifiedTarget);
        List<Address> dnsAddresses = new ArrayList<Address>();
        if (generateARecords) {
            dnsAddresses = Location.toAddresses(DnsManager.DNS_ADDRESS, all);
        }
        writeServerYaml(c, all, "all_servers", dnsAddresses, qualifiedTarget);
    }

    /**
     * my-id : [ { :name: my-fqdn, :ipv4: 1.1.1.1 }, ... ]
     */
    void writeServerYaml(YamlConfiguration c, List<Location> all, String id, List<Address> addresses,
            String qualifiedTarget)
        throws IOException {
        c.startArray(id);
        if (addresses != null) {
            for (Address a : addresses) {
                c.nextElement();
                writeAddress(c, all, a.getAddress(), a.getPort(), qualifiedTarget);
            }
        }
        c.endArray();
    }

    void writeAddress(YamlConfiguration c, List<Location> all, String address, int port,
            String qualifiedTarget) throws IOException {
        String host = getHostname(all, address);
        if (host != null) {
            if (qualifiedTarget != null) {
                c.write(YML_NAME, qualifiedTarget);
            } else {
                c.write(YML_NAME, host);
            }
            c.write(":ipv4", address);
            c.write(YML_PORT, port);
        }
    }

    void writeSrvRecord(YamlConfiguration c, DnsSrvRecord srv, String qualifiedTarget) throws IOException {
        c.write(":proto", srv.getProtocol());
        c.write(":lhs", srv.getLeftHandSide());
        c.write(":resource", srv.getResource());
        c.write(":priority", srv.getPriority());
        c.write(":weight", srv.getWeight());
        c.write(YML_PORT, srv.getPort());
        if (qualifiedTarget != null) {
            c.write(YML_TARGET, qualifiedTarget);
        } else {
            c.write(YML_TARGET, srv.getDestination());
        }
        c.write(":host", srv.getHost());
    }

    String getHostname(List<Location> locations, String ip) {
        for (Location l : locations) {
            if (ip.equals(l.getAddress())) {
                return l.getHostname();
            }
        }
        LOG.warn("No hostname found for " + ip + ", could be unmanged service");
        return null;
    }

    public void setDnsManager(DnsManager dnsManager) {
        m_dnsManager = dnsManager;
    }

    @Required
    public void setRegionManager(RegionManager regionManager) {
        m_regionManager = regionManager;
    }
}
