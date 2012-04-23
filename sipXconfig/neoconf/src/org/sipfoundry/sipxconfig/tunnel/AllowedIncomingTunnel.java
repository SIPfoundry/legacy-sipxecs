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

/**
 * Ends up in stunnel server-side settings. This accepts traffic on port on external interface and
 * sends it to port on localhost
 */
public class AllowedIncomingTunnel extends AbstractTunnel {
    private int m_allowedConnectionsPort;
    private RemoteOutgoingTunnel m_outgoingTunnel;

    public AllowedIncomingTunnel(String name) {
        super(name);
    }

    public int getAllowedConnectionsPort() {
        return m_allowedConnectionsPort;
    }

    public void setAllowedConnectionsPort(int remoteConnectPort) {
        m_allowedConnectionsPort = remoteConnectPort;
    }

    public RemoteOutgoingTunnel getOutgoingTunnel() {
        return m_outgoingTunnel;
    }

    public void setOutgoingTunnel(RemoteOutgoingTunnel outgoingTunnel) {
        m_outgoingTunnel = outgoingTunnel;
    }
}
