package org.sipfoundry.sipxconfig.mongo;

import static org.junit.Assert.assertEquals;

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
        assertEquals(0, server.size());
    }

    @Test
    public void tunnelsForTwoLocations() {
        MongoTunnelProvider provider = new MongoTunnelProvider();
        Collection<Location> otherLocations = Collections.singleton(new Location());
        Location thisLocation = new Location();
        Collection<RemoteOutgoingTunnel> client = provider.getClientSideTunnels(otherLocations, thisLocation);        
        assertEquals(2, client.size());
        Collection<AllowedIncomingTunnel> server = provider.getServerSideTunnels(otherLocations, thisLocation);        
        assertEquals(1, server.size());
    }
}
