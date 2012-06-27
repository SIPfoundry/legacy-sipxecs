/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.dns;

import junit.framework.TestCase;

public class DnsTestContextTest extends TestCase {
    DnsTestContextImpl m_dnsTestContext;
    
    public void testNop() {
        assertTrue(true);
    }

//    @Override
//    protected void setUp() {
//        ExternalCommandContext commandContext = EasyMock.createMock(ExternalCommandContext.class);
//        String command = getClass().getClassLoader().getResource(
//                "org/sipfoundry/sipxconfig/dns/mock-dns-test.sh").getFile();
//        m_dnsTestContext = new DnsTestContextImpl(commandContext, command);
//
//        CoreContext coreContext = createMock(CoreContext.class);
//        coreContext.getDomainName();
//        expectLastCall().andReturn("example.org").anyTimes();
//        replay(coreContext);
//
//        Location l1 = new Location();
//        l1.setFqdn("test1.example.org");
//        l1.setAddress("1.2.3.4");
//        Location l2 = new Location();
//        l2.setFqdn("test2.example.org");
//        l2.setAddress("4.3.2.1");
//        List<Location> locations = Arrays.asList(l1, l2);
//        
//        Address[][] addresses = new Address[][] { 
//                new Address[] { new Address(ProxyManager.TCP_ADDRESS, "tcp1", 5061), new Address(ProxyManager.TCP_ADDRESS, "tcp2", 5061)},
//                new Address[] { new Address(ProxyManager.UDP_ADDRESS, "udp1", 5061), new Address(ProxyManager.UDP_ADDRESS, "udp2", 5061)},
//                new Address[] { new Address(ProxyManager.TLS_ADDRESS, "tls1", 5062), new Address(ProxyManager.TLS_ADDRESS, "tls2", 5062)},
//                new Address[] { new Address(ImManager.XMPP_ADDRESS, "xmpp1", 106), new Address(ImManager.XMPP_ADDRESS, "xmpp2", 106)}
//        };
//        
//        AddressManager addressManager = createMock(AddressManager.class);
//        m_dnsTestContext.setAddressManager(addressManager);
//        addressManager.getAddresses(ProxyManager.TCP_ADDRESS);
//        expectLastCall().andReturn(Arrays.asList(addresses[0])).once();
//        addressManager.getAddresses(ProxyManager.UDP_ADDRESS);
//        expectLastCall().andReturn(Arrays.asList(addresses[1])).once();
//        addressManager.getAddresses(ProxyManager.TLS_ADDRESS);
//        expectLastCall().andReturn(Arrays.asList(addresses[2])).once();
//        addressManager.getAddresses(ImManager.XMPP_ADDRESS);
//        expectLastCall().andReturn(Arrays.asList(addresses[3])).once();
//        replay(addressManager);
//        
//        FeatureManager featureManager = createMock(FeatureManager.class);
//        featureManager.getLocationsForEnabledFeature(ProxyManager.FEATURE);
//        expectLastCall().andReturn(locations).once();
//        featureManager.getLocationsForEnabledFeature(ImManager.FEATURE);
//        expectLastCall().andReturn(null).once();
//        replay(featureManager);
//        m_dnsTestContext.setFeatureManager(featureManager);
//        m_dnsTestContext.setCoreContext(coreContext);
//    }
//
//    public void testValid() {
//        m_dnsTestContext.execute(false);
//        assertEquals("DNS Records", m_dnsTestContext.getResult());
//    }
//
//    public void testInvalid() {
//        m_dnsTestContext.execute(true);
//        assertEquals("DNS Configuration ERROR", m_dnsTestContext.getResult());
//    }
}
