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
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class TunnelConfigurationTest {

    @Test
    public void configClientConfig() throws IOException {
        TunnelConfiguration config = new TunnelConfiguration();
        config.setVelocityEngine(TestHelper.getVelocityEngine());

        RemoteOutgoingTunnel t1 = new RemoteOutgoingTunnel("t1");
        t1.setLocalhostPort(1234);
        t1.setRemoteMachineAddress("t1.example.org");
        t1.setPortOnRemoteMachine(4321);

        RemoteOutgoingTunnel t2 = new RemoteOutgoingTunnel("t2");
        t2.setLocalhostPort(2345);
        t2.setRemoteMachineAddress("t2.example.org");
        t2.setPortOnRemoteMachine(5432);

        List<RemoteOutgoingTunnel> tunnels = Arrays.asList(t1, t2);

        StringWriter actual = new StringWriter();
        config.writeOutgoingTunnels(actual, tunnels);
        String expected = IOUtils.toString(getClass().getResourceAsStream("stunnel-client.conf"));
        assertEquals(expected, actual.toString());
    }
    
    @Test
    public void configServer() throws IOException {
        TunnelConfiguration config = new TunnelConfiguration();
        config.setVelocityEngine(TestHelper.getVelocityEngine());

        AllowedIncomingTunnel t1 = new AllowedIncomingTunnel("t1");
        t1.setLocalhostPort(4321);
        t1.setAllowedConnectionsPort(1234);
        AllowedIncomingTunnel t2 = new AllowedIncomingTunnel("t2");                
        t2.setLocalhostPort(2345);
        t2.setAllowedConnectionsPort(5432);
        List<AllowedIncomingTunnel> tunnels = Arrays.asList(t1, t2);

        StringWriter actual = new StringWriter();
        config.writeIncomingTunnels(actual, tunnels);
        
        String expected = IOUtils.toString(getClass().getResourceAsStream("stunnel-server.conf"));
        assertEquals(expected, actual.toString());
        
    }
}
