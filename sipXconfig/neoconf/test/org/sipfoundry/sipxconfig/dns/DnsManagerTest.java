/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dns;


import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.registrar.Registrar;

public class DnsManagerTest implements DnsProvider {
    private DnsManagerImpl m_dns;
    private AddressType m_any = new AddressType("any");
    private Location m_l1 = new Location("one", "1");
    private Location m_l2 = new Location("two", "2");
    private Location m_l3 = new Location("three", "3");
    private Address m_a1 = new Address(m_any, "1");
    private Address m_a2 = new Address(m_any, "2");
    private Address m_a3 = new Address(m_any, "3");
    private DnsProvider m_dummyProvider = this;
    
    @Before
    public void setUp() {
        m_dns = new DnsManagerImpl();
        m_dns.setProviders(Arrays.asList(m_dummyProvider));
    }
    
    @Test
    public void singleAddress() {
        assertNull(m_dns.getSingleAddress(m_any, null, m_l1));
        assertEquals(m_a1, m_dns.getSingleAddress(m_any, Arrays.asList(m_a1), m_l1));
        assertEquals(m_a1, m_dns.getSingleAddress(m_any, Arrays.asList(m_a1), m_l2));
        assertEquals(m_a1, m_dns.getSingleAddress(m_any, Arrays.asList(m_a1, m_a2, m_a3), null));
        assertEquals(m_a1, m_dns.getSingleAddress(m_any, Arrays.asList(m_a1, m_a2, m_a3), m_l1));
        assertEquals(m_a2, m_dns.getSingleAddress(m_any, Arrays.asList(m_a1, m_a2, m_a3), m_l2));
        assertEquals(m_a1, m_dns.getSingleAddress(m_any, Arrays.asList(m_a1, m_a2), m_l3));
    }
    
    @Test
    public void singleRRAddress() {
        DnsProvider provider = new DnsProvider() {

            @Override
            public Address getAddress(DnsManager manager, AddressType t, Collection<Address> addresses,
                    Location whoIsAsking) {
                return m_a3;
            }

            @Override
            public List<ResourceRecords> getResourceRecords(DnsManager manager, Location whoIsAsking) {
                return null;
            }
        };            
        m_dns.setProviders(Arrays.asList(provider));
        assertNull(m_dns.getSingleAddress(m_any, null, m_l1));
        assertEquals(m_a1, m_dns.getSingleAddress(m_any, Arrays.asList(m_a1), m_l1));
        assertEquals(m_a3, m_dns.getSingleAddress(Registrar.TCP_ADDRESS, Arrays.asList(m_a1, m_a2), m_l2));
    }

    @Override
    public Address getAddress(DnsManager manager, AddressType t, Collection<Address> addresses, Location whoIsAsking) {
        return null;
    }

    @Override
    public List<ResourceRecords> getResourceRecords(DnsManager manager, Location whoIsAsking) {
        return null;
    }
}
