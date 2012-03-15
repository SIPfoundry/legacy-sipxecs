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
package org.sipfoundry.sipxconfig.mongo;

import static org.junit.Assert.assertEquals;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.tunnel.AllowedIncomingTunnel;
import org.sipfoundry.sipxconfig.tunnel.RemoteOutgoingTunnel;


public class MongoTunnelProviderTest {
    
    @Test
    public void noTunnelsForSingleLocation() {
        MongoTunnelProvider provider = new MongoTunnelProvider();
        Collection<Location> otherLocations = Collections.emptyList();
        Location thisLocation = new Location();
        Collection<RemoteOutgoingTunnel> client = provider.getClientSideTunnels(otherLocations, thisLocation);        
        assertEquals(0, client.size());
        Collection<AllowedIncomingTunnel> server = provider.getServerSideTunnels(otherLocations, thisLocation);        
        assertEquals(1, server.size());
        thisLocation = new Location();
        thisLocation.setPrimary(true);
        client = provider.getClientSideTunnels(otherLocations, thisLocation);        
        assertEquals(1, client.size());
    }

    @Test
    public void tunnelsForTwoLocations() {
        MongoTunnelProvider provider = new MongoTunnelProvider();
        Location primary = new Location();
        primary.setPrimary(true);
        Collection<Location> otherLocations = Arrays.asList(new Location[] {new Location(), primary, new Location()});
        Location thisLocation = new Location();
        RemoteOutgoingTunnel[] client = provider.getClientSideTunnels(otherLocations, thisLocation).toArray(new RemoteOutgoingTunnel[0]);
        assertEquals(3, client.length);
        assertEquals(27020, client[0].getLocalhostPort());
        assertEquals(27019, client[1].getLocalhostPort());
        assertEquals(27021, client[2].getLocalhostPort());
        Collection<AllowedIncomingTunnel> server = provider.getServerSideTunnels(otherLocations, thisLocation);        
        assertEquals(1, server.size());
    }
}
