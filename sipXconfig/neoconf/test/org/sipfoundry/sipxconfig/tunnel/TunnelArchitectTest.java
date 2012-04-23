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
package org.sipfoundry.sipxconfig.tunnel;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.util.Arrays;

import org.junit.Test;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;

public class TunnelArchitectTest {

    @Test
    public void build() throws IOException {
        TunnelArchitect architect = new TunnelArchitect();
        architect.setServerStartPort(1000);
        architect.setClientStartPort(2000);
        
        Location l1 = new Location("one", "1.1.1.1");
        l1.setUniqueId(1);
        Location l2 = new Location("two", "2.2.2.2");
        l2.setUniqueId(2);
        AddressType t1 = new AddressType("t1", 100);
        AddressType t2 = new AddressType("t2", 200);
        DefaultFirewallRule r1 = new DefaultFirewallRule(t1);
        DefaultFirewallRule r2 = new DefaultFirewallRule(t2);
        Address a1 = new Address(t1, l1.getAddress());
        Address a2 = new Address(t1, l2.getAddress());        
        Address a3 = new Address(t2, l1.getAddress());
        Address a4 = new Address(t2, l2.getAddress());        
        AddressManager addressManager = createMock(AddressManager.class);
        addressManager.getAddresses(t1, l1);
        expectLastCall().andReturn(Arrays.asList(a1, a2)).once();
        addressManager.getAddresses(t2, l1);
        expectLastCall().andReturn(Arrays.asList(a3, a4)).once();        
        addressManager.getAddresses(t1, l2);
        expectLastCall().andReturn(Arrays.asList(a1, a2)).once();
        addressManager.getAddresses(t2, l2);
        expectLastCall().andReturn(Arrays.asList(a3, a4)).once();        
        replay(addressManager);        
        architect.setAddressManager(addressManager);

        architect.build(Arrays.asList(l1, l2), Arrays.asList(r1, r2));
        AllowedIncomingTunnel[] in1 = architect.getAllowedIncomingTunnels(l1).toArray(new AllowedIncomingTunnel[0]);
        assertEquals(2, in1.length);
        assertEquals("t1-1", in1[0].getName());
        assertEquals(100, in1[0].getLocalhostPort());
        assertEquals(1000, in1[0].getAllowedConnectionsPort());
        assertEquals("t2-1", in1[1].getName());
        assertEquals(200, in1[1].getLocalhostPort());
        assertEquals(1001, in1[1].getAllowedConnectionsPort());

        AllowedIncomingTunnel[] in2 = architect.getAllowedIncomingTunnels(l2).toArray(new AllowedIncomingTunnel[0]);
        assertEquals(2, in2.length);
        assertEquals("t1-2", in2[0].getName());
        assertEquals(100, in2[0].getLocalhostPort());
        assertEquals(1000, in2[0].getAllowedConnectionsPort());
        assertEquals("t2-2", in2[1].getName());
        assertEquals(200, in2[1].getLocalhostPort());
        assertEquals(1001, in2[1].getAllowedConnectionsPort());
        
        RemoteOutgoingTunnel[] out1 = architect.getRemoteOutgoingTunnels(l1).toArray(new RemoteOutgoingTunnel[0]);
        assertEquals(2, out1.length);
        assertEquals("t1-2", out1[0].getName());
        assertEquals(2002, out1[0].getLocalhostPort());
        assertEquals(1000, out1[0].getPortOnRemoteMachine());
        assertEquals(l2.getAddress(), out1[0].getRemoteMachineAddress());        
        
        verify(addressManager);        
    }
}
