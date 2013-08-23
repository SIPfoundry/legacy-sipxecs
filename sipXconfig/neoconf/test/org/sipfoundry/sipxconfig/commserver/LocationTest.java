/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.commserver;

import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManagerImpl;

import junit.framework.TestCase;

public class LocationTest extends TestCase {
    public void testGetProcessMonitorUrl() {
        Location out = new Location();
        out.setFqdn("localhost");
        assertEquals("https://localhost:8092/RPC2", out.getProcessMonitorUrl());
        Location out1 = new Location();
        out1.setFqdn("mysystem.europe.pmd.com");
        assertEquals("https://mysystem.europe.pmd.com:8092/RPC2", out1.getProcessMonitorUrl());
    }

    @SuppressWarnings("deprecation")
    public void testParseAddress() {
        // it tests setUrl - which is deprecated, because it's only used when migrating from
        // topology.xml
        Location out = new Location();
        out.setUrl("https://localhost:8091/cgi-bin/replication/replication.cgi");
        assertEquals("localhost", out.getFqdn());
        assertNull(out.getAddress());

        Location out1 = new Location();
        out1.setUrl("https://192.168.1.10:8091/cgi-bin/replication/replication.cgi");
        assertEquals("192.168.1.10", out1.getFqdn());
        assertNull(out.getAddress());
    }

    public void testGetHostnameInSipDomain() {
        Domain d = new Domain("openuctest.ezuce.ro");
        d.setNetworkName("ezuce.ro");
        new DomainManagerImpl().setTestDomain(d);
        Location location = new Location();
        location.setFqdn("openuctest.ezuce.ro");
        assertEquals("openuctest", location.getHostname());
        assertEquals("openuctest.ezuce.ro", location.getHostnameInSipDomain());

        d = new Domain("sip.ezuce.ro");
        d.setNetworkName("sip.ezuce.ro");
        new DomainManagerImpl().setTestDomain(d);
        Location location1 = new Location();
        location1.setFqdn("openuctest.sip.ezuce.ro");
        assertEquals("openuctest", location1.getHostname());
        assertEquals("openuctest.sip.ezuce.ro", location1.getHostnameInSipDomain());

        d = new Domain("ezuce.ro");
        d.setNetworkName("ezuce.ro");
        new DomainManagerImpl().setTestDomain(d);
        Location location2 = new Location();
        location2.setFqdn("openuctest.ezuce.ro");
        assertEquals("openuctest", location2.getHostname());
        assertEquals("openuctest.ezuce.ro", location2.getHostnameInSipDomain());

        d = new Domain("ezuce.ro");
        d.setNetworkName("ezuce.ro");
        new DomainManagerImpl().setTestDomain(d);
        Location location3 = new Location();
        location3.setFqdn("openuctest.sample.ezuce.ro");
        assertEquals("openuctest.sample", location3.getHostname());
        assertEquals("openuctest.sample.ezuce.ro", location3.getHostnameInSipDomain());

        d = new Domain("sip.ezuce.ro");
        d.setNetworkName("network.ezuce.ro");
        new DomainManagerImpl().setTestDomain(d);
        Location location4 = new Location();
        location4.setFqdn("openuctest.network.ezuce.ro");
        assertEquals("openuctest", location4.getHostname());
        assertEquals("openuctest.sip.ezuce.ro", location4.getHostnameInSipDomain());

        d = new Domain("sip.ezuce.ro");
        d.setNetworkName("ezuce.ro");
        new DomainManagerImpl().setTestDomain(d);
        Location location5 = new Location();
        location5.setFqdn("openuctest.other.ezuce.ro");
        assertEquals("openuctest.other", location5.getHostname());
        assertEquals("openuctest.other.sip.ezuce.ro", location5.getHostnameInSipDomain());
    }
}
