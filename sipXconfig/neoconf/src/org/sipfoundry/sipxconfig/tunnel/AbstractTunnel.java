/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.tunnel;

/**
 * Common code for server and client tunnel settings
 *
 * Example:
 *   For a service Foo that bind to 127.0.0.1 interface on port X might have
 *
 *   AllowedIncomingTunnel
 *      localhostPort = X
 *      allowedConnectionsPort = Y
 *
 *   RemoteOutgoingTunnel
 *      localhostPort = Z
 *      portOnRemoteMachine = Y
 *
 *  Allowing code on remote machine to connect remote service Foo to port Z on localhost
 */
public class AbstractTunnel {
    private String m_name;
    private int m_localhostPort;

    public AbstractTunnel(String name) {
        m_name = name;
    }

    public String getName() {
        return m_name;
    }

    public int getLocalhostPort() {
        return m_localhostPort;
    }

    public void setLocalhostPort(int localhostPort) {
        m_localhostPort = localhostPort;
    }
}
