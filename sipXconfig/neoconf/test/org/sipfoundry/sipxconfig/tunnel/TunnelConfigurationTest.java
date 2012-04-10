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

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Arrays;
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
