/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.tunnel;

/**
 * Ends up in stunnel server-side settings. This accepts traffic on port on external interface and
 * sends it to port on localhost
 */
public class AllowedIncomingTunnel extends AbstractTunnel {
    private int m_allowedConnectionsPort;

    public AllowedIncomingTunnel(String name) {
        super(name);
    }

    public int getAllowedConnectionsPort() {
        return m_allowedConnectionsPort;
    }

    public void setAllowedConnectionsPort(int remoteConnectPort) {
        m_allowedConnectionsPort = remoteConnectPort;
    }
}
