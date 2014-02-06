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

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.cfgmgt.YamlConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.region.Region;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class DnsConfigTest {
    private DnsConfig m_config;
    private Location m_l1;
    private Location m_l2;
    private Location m_l3;
    private List<Location> m_locations;
    private Address m_a1; 
    private Address m_a2;
    private Address m_a3;      
    private Domain m_d = new Domain("x");
    
    @Before
    public void setUp() {
        TestHelper.initDefaultDomain();
        m_config = new DnsConfig();
        m_l1 = new Location("one.example.org", "1.1.1.1");
        m_l2 = new Location("two.example.org", "2.2.2.2");
        m_l3 = new Location("three.example.org", "3.3.3.3");
        m_locations = Arrays.asList(m_l1, m_l2, m_l3);
        m_a1 = new Address(DnsManager.DNS_ADDRESS, "1.1.1.1");
        m_a2 = new Address(DnsManager.DNS_ADDRESS, "2.2.2.2");
        m_a3 = new Address(DnsManager.DNS_ADDRESS, "3.3.3.3");
        m_d.setNetworkName("x");
    }
    
    @Test
    public void externalDns() {
        List<?> nameservers = sun.net.dns.ResolverConfiguration.open().nameservers(); 
        for( Object dns : nameservers ) { 
            System.out.print( dns + " " ); 
        } 
    }
    
    @Test
    public void cfdat() throws IOException {
//        Region r0 = Region.DEFAULT;
        DnsView v0 = new DnsView(Region.DEFAULT.getName());
        DnsView v1 = new DnsView("r1");
//        Region r1 = new Region("r1");
        Collection<DnsView> views = Arrays.asList(v0, v1);
        DnsSettings settings = new DnsSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        StringWriter actual = new StringWriter();
        m_config.writeSettings(actual, true, true, views, settings);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-named.cfdat"));
        assertEquals(expected, actual.toString());        
    }
    
    @Test
    public void resolv() throws IOException {
        StringWriter actual = new StringWriter();
        m_config.writeResolv(actual, m_l1, "x", Arrays.asList(m_a1, m_a2));
        assertEquals("search x\nnameserver 1.1.1.1\nnameserver 2.2.2.2\n", actual.toString());
        // give priority to local dns server
        actual = new StringWriter();
        m_config.writeResolv(actual, m_l2, "x", Arrays.asList(m_a1, m_a2));
        assertEquals("search x\nnameserver 2.2.2.2\nnameserver 1.1.1.1\n", actual.toString());
    }

    @Test
    public void emptyZone() throws IOException {
        StringWriter actual = new StringWriter();
        m_config.writeZoneConfig(actual, m_d, m_locations, null, null, 1, null);
        String expected = IOUtils.toString(getClass().getResourceAsStream("empty-zone.yml"));
        assertEquals(expected, actual.toString());
    }

    @Test
    public void fullZone() throws IOException {
        StringWriter actual = new StringWriter();
        DnsSrvRecord[] records = new DnsSrvRecord[] {
                DnsSrvRecord.domainLevel("_sip._tcp", "rr1", 1, "s1"),
                DnsSrvRecord.hostLevel("_sip._tcp", "rr2", "h1", 2, "s1"),
                DnsSrvRecord.hostLevel("_sip._tcp", "rr3", "h2", 3, "s1"),
        };
        List<Address> all = Arrays.asList(m_a1, m_a2, m_a3);
        List<DnsSrvRecord> rrs = Arrays.asList(records);
        m_config.writeZoneConfig(actual, m_d, m_locations, all, rrs, 1, null);
        String expected = IOUtils.toString(getClass().getResourceAsStream("full-zone.yml"));
        assertEquals(expected, actual.toString());

        DnsView.ExcludedRecords[] excludeAll = new DnsView.ExcludedRecords[] {
            DnsView.ExcludedRecords.A, DnsView.ExcludedRecords.NAPTR, DnsView.ExcludedRecords.NS
        };
        actual = new StringWriter();
        m_config.writeZoneConfig(actual, m_d, m_locations, all, rrs, 1, excludeAll);
        expected = IOUtils.toString(getClass().getResourceAsStream("full-zone-with-excludes.yml"));
        assertEquals(expected, actual.toString());
    }

    @Test
    public void fullZoneSipDomainDifferentThanNetwork() throws IOException {
        StringWriter actual = new StringWriter();
        DnsSrvRecord[] records = new DnsSrvRecord[] {
                DnsSrvRecord.domainLevel("_sip._tcp", "rr1", 1, "s1"),
                DnsSrvRecord.hostLevel("_sip._tcp", "rr2", "h1", 2, "s1"),
                DnsSrvRecord.hostLevel("_sip._tcp", "rr3", "h2", 3, "s1"),
        };
        List<Address> all = Arrays.asList(m_a1, m_a2, m_a3);
        List<DnsSrvRecord> rrs = Arrays.asList(records);
        m_config.writeZoneConfig(actual, m_d, m_locations, all, rrs, 1, null);
        String expected = IOUtils.toString(getClass().getResourceAsStream("full-zone-sip-domain-not-network.yml"));
        assertEquals(expected, actual.toString());
    }

    @Test
    public void fullZoneSameFqdnAsDomain() throws IOException {
        StringWriter actual = new StringWriter();
        DnsSrvRecord[] records = new DnsSrvRecord[] {
                DnsSrvRecord.domainLevel("_sip._tcp", "rr1", 1, "s1")
        };
        List<Address> all = Arrays.asList(m_a1);
        List<DnsSrvRecord> rrs = Arrays.asList(records);
        Domain d = new Domain("one.example.org");
        d.setNetworkName("one.example.org");
        m_config.writeZoneConfig(actual, d, Arrays.asList(m_l1), all, rrs, 1, null);
        String expected = IOUtils.toString(getClass().getResourceAsStream("full-zone-domain-fqdn.yml"));
        assertEquals(expected, actual.toString());
    }
    
    @Test
    public void writeServerYaml() throws IOException {
        StringWriter actual = new StringWriter();
        YamlConfiguration c = new YamlConfiguration(actual);
        m_config.writeServerYaml(c, m_locations, "robin", null, null, true);
        assertEquals("robin:\n", actual.toString());
        
        actual = new StringWriter();
        c = new YamlConfiguration(actual);
        m_config.writeServerYaml(c, m_locations, "robin", Collections.singletonList(m_a1), null, true);
        assertEquals("robin:\n - :name: one\n   :ipv4: 1.1.1.1\n   :port: 0\n", actual.toString());

        actual = new StringWriter();
        c = new YamlConfiguration(actual);
        m_config.writeServerYaml(c, m_locations, "robin", Collections.singletonList(m_a1), "one.example.org", true);
        assertEquals("robin:\n - :name: one.example.org\n   :ipv4: 1.1.1.1\n   :port: 0\n", actual.toString());

        actual = new StringWriter();
        c = new YamlConfiguration(actual);
        m_config.writeServerYaml(c, m_locations, "robin", Arrays.asList(m_a1, m_a2), null, true);   
        String expected = IOUtils.toString(getClass().getResourceAsStream("server.yml"));
        assertEquals(expected, actual.toString());
    }

    @Test
    public void named() throws IOException {
        Domain d = new Domain("x");
        Region r0 = Region.DEFAULT;        
        Region r1 = new Region("r1");
        r1.setUniqueId(1);
        r1.setAddresses(new String[] {"1.1.1.1", "1.1.1.2"});
        Region r2 = new Region("r2");
        r2.setUniqueId(2);
        r2.setAddresses(new String[] {"2.2.2.1", "2.2.2.2"});
        Collection<Address> forwarders = Arrays.asList(
                new Address(DnsManager.DNS_ADDRESS, "8.8.8.8"),
                new Address(DnsManager.DNS_ADDRESS, "8.8.8.2")
        );
        DnsView v0 = new DnsView(r0.getName());
        v0.setRegionId(r0.getId());
        DnsView v1 = new DnsView(r1.getName());
        v1.setRegionId(r1.getId());
        DnsView v2 = new DnsView(r2.getName());
        v2.setRegionId(r2.getId());
        
        
        StringWriter actual = new StringWriter();
        m_config.writeNamedConfig(actual, d, Arrays.asList(v0, v1, v2), forwarders, Arrays.asList(r0, r1, r2));
        String expected = IOUtils.toString(getClass().getResourceAsStream("named.expected.yml"));
        assertEquals(expected, actual.toString());
    }
}
