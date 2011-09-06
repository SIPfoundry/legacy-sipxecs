/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mongo;

import static org.junit.Assert.assertEquals;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import org.junit.Test;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
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
