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

public class TunnelClientConfigurationFileTest extends TunnelConfigurationTestBase {

    @Test
    public void config() throws IOException {
        TunnelClientConfigurationFile f = new TunnelClientConfigurationFile();
        f.setTemplate("tunnel/tunnel-client.conf.vm");
        f.setVelocityEngine(TestHelper.getVelocityEngine());

        RemoteOutgoingTunnel t1 = new RemoteOutgoingTunnel("t1");
        t1.setLocalhostPort(1234);
        t1.setRemoteMachineAddress("t1.example.org");
        t1.setPortOnRemoteMachine(4321);

        RemoteOutgoingTunnel t2 = new RemoteOutgoingTunnel("t2");
        t2.setLocalhostPort(2345);
        t2.setRemoteMachineAddress("t2.example.org");
        t2.setPortOnRemoteMachine(5432);

        Collection<RemoteOutgoingTunnel> tunnels = Arrays.asList(new RemoteOutgoingTunnel[] {
            t1, t2
        });
        TunnelProvider p = createMock(TunnelProvider.class);
        p.getClientSideTunnels(eq(Collections.singletonList(m_remoteLocation)), same(m_thisLocation));
        expectLastCall().andReturn(tunnels).once();
        replay(p);

        m_tunnelManager.setProviders(Collections.singletonList(p));

        f.setTunnelManager(m_tunnelManager);
        f.setLocationsManager(m_locationManager);

        StringWriter actual = new StringWriter();
        f.write(actual, m_thisLocation);

        verify(p);

        InputStream expected = getClass().getResourceAsStream("stunnel-client.conf");
        assertEquals(IOUtils.toString(expected), actual.toString());
    }
}
