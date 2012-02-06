/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dns;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.commserver.Location;

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
        m_config = new DnsConfig();
        m_l1 = new Location("one", "1.1.1.1");
        m_l2 = new Location("two", "2.2.2.2");
        m_l3 = new Location("three", "3.3.3.3");
        m_locations = Arrays.asList(m_l1, m_l2, m_l3);
        m_a1 = new Address("1.1.1.1");
        m_a2 = new Address("2.2.2.2");
        m_a3 = new Address("3.3.3.3");
    }

    @Test
    public void emptyZone() throws IOException {
        StringWriter actual = new StringWriter();
        m_config.writeZoneConfig(actual, "x", m_locations, null, null, null, null, 1);
        String expected = IOUtils.toString(getClass().getResourceAsStream("empty-zone.yml"));
        assertEquals(expected, actual.toString());
    }

    @Test
    public void fullZone() throws IOException {
        StringWriter actual = new StringWriter();
        List<Address> all = Arrays.asList(m_a1, m_a2, m_a3);
        m_config.writeZoneConfig(actual, "x", m_locations, all, all, all, all, 1);
        String expected = IOUtils.toString(getClass().getResourceAsStream("full-zone.yml"));
        assertEquals(expected, actual.toString());
    }
    
    @Test
    public void writeServerYaml() throws IOException {
        StringWriter actual = new StringWriter();
        m_config.writeServerYaml(actual, m_locations, "robin", null);
        assertEquals("robin:[]", ws(actual.toString()));
        
        actual = new StringWriter();
        m_config.writeServerYaml(actual, m_locations, "robin", Collections.singletonList(m_a1));
        assertEquals("robin:[{:name:one,:ipv4:1.1.1.1}]", ws(actual.toString()));

        actual = new StringWriter();
        m_config.writeServerYaml(actual, m_locations, "robin", Arrays.asList(m_a1, m_a2));        
        assertEquals("robin:[{:name:one,:ipv4:1.1.1.1},{:name:two,:ipv4:2.2.2.2}]", ws(actual.toString()));
    }
    
    public String ws(String s) {
        return s.replaceAll("\\s*", "");        
    }
}
