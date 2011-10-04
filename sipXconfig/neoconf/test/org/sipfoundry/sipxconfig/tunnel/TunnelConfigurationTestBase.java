package org.sipfoundry.sipxconfig.tunnel;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.same;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertArrayEquals;

import java.io.IOException;
import java.util.Collections;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;

public abstract class TunnelConfigurationTestBase {
    protected TunnelManagerImpl m_tunnelManager;
    protected LocationsManager m_locationManager;
    protected Location m_thisLocation;
    protected Location m_remoteLocation;
    
    @Before
    public void setupTunnelManager() {
        m_tunnelManager = new TunnelManagerImpl() {
            
            @Override
            protected SipxReplicationContext getSipxReplicationContext() {
                // TODO Auto-generated method stub
                return null;
            }
        };
        m_locationManager = createMock(LocationsManager.class);        
        m_thisLocation = new Location();
        m_thisLocation.setName("this");
        m_remoteLocation = new Location();
        m_remoteLocation.setName("remote");
        Location[] locations = new Location[] {m_thisLocation, m_remoteLocation};
        m_locationManager.getLocations();        
        expectLastCall().andReturn(locations).anyTimes();                
        replay(m_locationManager);        
        m_tunnelManager.setLocationsManager(m_locationManager);
    }
    
    @After
    public void verifyTunnelManager() {
        verify(m_locationManager);        
    }
    
    abstract void config() throws IOException;

    /*
    @Test
    public void remoteOutgoingTunnels() {
        RemoteOutgoingTunnel t1 = new RemoteOutgoingTunnel("t1");
        RemoteOutgoingTunnel t2 = new RemoteOutgoingTunnel("t2");                
        TunnelProvider p = createMock(TunnelProvider.class);
        p.getClientSideTunnels(eq(Collections.singletonList(m_remoteLocation)), same(m_thisLocation));
        expectLastCall().andReturn(Collections.singleton(t1)).once();                        
        p.getClientSideTunnels(eq(Collections.singletonList(m_thisLocation)), same(m_remoteLocation));
        expectLastCall().andReturn(Collections.singleton(t2)).once();                        
        replay(p);
        
        m_tunnelManager.setProviders(Collections.singletonList(p));        
        assertArrayEquals(new Object[] {t1, t2}, m_tunnelManager.getRemoteOutgoingTunnels().toArray());
        
        verify(p);
    }
    
    @Test
    public void allowIncomingTunnels() {
        AllowedIncomingTunnel t1 = new AllowedIncomingTunnel("t1");
        AllowedIncomingTunnel t2 = new AllowedIncomingTunnel("t2");                
        TunnelProvider p = createMock(TunnelProvider.class);
        p.getServerSideTunnels(eq(Collections.singletonList(m_remoteLocation)), same(m_thisLocation));
        expectLastCall().andReturn(Collections.singleton(t1)).once();                        
        p.getServerSideTunnels(eq(Collections.singletonList(m_thisLocation)), same(m_remoteLocation));
        expectLastCall().andReturn(Collections.singleton(t2)).once();                        
        replay(p);
        
        m_tunnelManager.setProviders(Collections.singletonList(p));        
        assertArrayEquals(new Object[] {t1, t2}, m_tunnelManager.getAllowedIncomingTunnels().toArray());
        
        verify(p);
    }
    */
}
