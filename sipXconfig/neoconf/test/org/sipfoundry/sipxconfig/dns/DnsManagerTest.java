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

import org.junit.Test;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.registrar.Registrar;


public class DnsManagerTest {
    private DnsManagerImpl dns = new DnsManagerImpl();    
    private AddressType any = new AddressType("any");
    private Location l1 = new Location("one", "1");
    private Location l2 = new Location("two", "2");
    private Location l3 = new Location("three", "3");
    private Address a1 = new Address(any, "1");
    private Address a2 = new Address(any, "2");
    private Address a3 = new Address(any, "3");    
    
    @Test
    public void singleAddress() {
        assertNull(dns.getSingleAddress(any, null, l1));
        assertEquals(a1, dns.getSingleAddress(any, Arrays.asList(a1), l1));
        assertEquals(a1, dns.getSingleAddress(any, Arrays.asList(a1), l2));
        assertEquals(a1, dns.getSingleAddress(any, Arrays.asList(a1, a2, a3), null));
        assertEquals(a1, dns.getSingleAddress(any, Arrays.asList(a1, a2, a3), l1));
        assertEquals(a2, dns.getSingleAddress(any, Arrays.asList(a1, a2, a3), l2));
        assertEquals(a1, dns.getSingleAddress(any, Arrays.asList(a1, a2), l3));
    }
    
    @Test
    public void singleRRAddress() {
        Address a1 = new Address(Registrar.TCP_ADDRESS, "1");
        Address a2 = new Address(Registrar.TCP_ADDRESS, "2");
        Address a3 = new Address(Registrar.TCP_ADDRESS, "3");
        assertNull(dns.getSingleAddress(Registrar.TCP_ADDRESS, null, l1));
        assertEquals(a1, dns.getSingleAddress(Registrar.TCP_ADDRESS, Arrays.asList(a1), l1));
        assertEquals(a1, dns.getSingleAddress(Registrar.TCP_ADDRESS, Arrays.asList(a1), l2));
        assertEquals(new Address(Registrar.TCP_ADDRESS, "rr.one"), dns.getSingleAddress(Registrar.TCP_ADDRESS, Arrays.asList(a1, a2, a3), l1));
    }
}
