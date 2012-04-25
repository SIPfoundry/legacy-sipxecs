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

import org.sipfoundry.sipxconfig.address.AddressType;

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
    private AddressType.Protocol m_protocol;

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

    public AddressType.Protocol getProtocol() {
        return m_protocol;
    }

    public void setProtocol(AddressType.Protocol protocol) {
        m_protocol = protocol;
    }
}
