package org.sipfoundry.sipxconfig.tunnel;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.same;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.InputStream;
import java.io.StringWriter;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.TestHelper;

public class TunnelServerConfigurationFileTest extends TunnelConfigurationTestBase {

    @Test
    public void config() throws IOException {
        TunnelServerConfigurationFile f = new TunnelServerConfigurationFile();
        f.setTemplate("tunnel/tunnel-server.conf.vm");
        f.setVelocityEngine(TestHelper.getVelocityEngine());        
        
        AllowedIncomingTunnel t1 = new AllowedIncomingTunnel("t1");
        t1.setLocalhostPort(4321);
        t1.setAllowedConnectionsPort(1234);
        AllowedIncomingTunnel t2 = new AllowedIncomingTunnel("t2");                
        t2.setLocalhostPort(2345);
        t2.setAllowedConnectionsPort(5432);
        Collection<AllowedIncomingTunnel> tunnels = Arrays.asList(new AllowedIncomingTunnel[] {t1, t2});
        TunnelProvider p = createMock(TunnelProvider.class);
        p.getServerSideTunnels(eq(Collections.singletonList(m_remoteLocation)), same(m_thisLocation));
        expectLastCall().andReturn(tunnels).once();                        
        replay(p);
        
        m_tunnelManager.setProviders(Collections.singletonList(p));        

        f.setTunnelManager(m_tunnelManager);
        f.setLocationsManager(m_locationManager);

        StringWriter actual = new StringWriter();
        f.write(actual, m_thisLocation);

        verify(p);

        InputStream expected = getClass().getResourceAsStream("stunnel-server.conf");
        assertEquals(IOUtils.toString(expected), actual.toString());
    }
}
