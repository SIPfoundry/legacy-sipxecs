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
import java.util.Collections;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.cfgmgt.YamlConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
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
        DnsSettings settings = new DnsSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        StringWriter actual = new StringWriter();
        m_config.writeSettings(actual, true, true, settings);
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
        m_config.writeZoneConfig(actual, "x", m_locations, null, null, null, null, 1, true);
        String expected = IOUtils.toString(getClass().getResourceAsStream("empty-zone.yml"));
        assertEquals(expected, actual.toString());
    }

    @Test
    public void fullZone() throws IOException {
        StringWriter actual = new StringWriter();
        List<Address> all = Arrays.asList(m_a1, m_a2, m_a3);
        ResourceRecords rr1 = new ResourceRecords("_sip._tcp", "rr1");
        rr1.addAddress(m_a1);
        ResourceRecords rr2 = new ResourceRecords("_xyx._abc", "rr2");
        rr2.addAddress(m_a2);
        List<ResourceRecords> rrs = Arrays.asList(rr1, rr2);
        m_config.writeZoneConfig(actual, "x", m_locations, all, all, all, rrs, 1, true);
        String expected = IOUtils.toString(getClass().getResourceAsStream("full-zone.yml"));
        assertEquals(expected, actual.toString());
    }
    
    @Test
    public void writeServerYaml() throws IOException {
        StringWriter actual = new StringWriter();
        YamlConfiguration c = new YamlConfiguration(actual);
        m_config.writeServerYaml(c, m_locations, "robin", null);
        assertEquals("robin:\n", actual.toString());
        
        actual = new StringWriter();
        c = new YamlConfiguration(actual);
        m_config.writeServerYaml(c, m_locations, "robin", Collections.singletonList(m_a1));
        assertEquals("robin:\n - :name: one.example.org\n   :ipv4: 1.1.1.1\n   :port: 0\n", actual.toString());

        actual = new StringWriter();
        c = new YamlConfiguration(actual);
        m_config.writeServerYaml(c, m_locations, "robin", Arrays.asList(m_a1, m_a2));   
        String expected = IOUtils.toString(getClass().getResourceAsStream("server.yml"));
        assertEquals(expected, actual.toString());
    }
}
