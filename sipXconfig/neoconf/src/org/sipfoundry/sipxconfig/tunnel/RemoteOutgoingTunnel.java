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
 * Used in stunnel client config, maps a local port to a remote port. Remote port should have a
 * corresponding AllowedIncomingTunnel config if this tunnel is to work
 */
public class RemoteOutgoingTunnel extends AbstractTunnel {
    private int m_portOnRemoteMachine;
    private String m_remoteMachineAddress;
    private AllowedIncomingTunnel m_incomingTunnel;

    public RemoteOutgoingTunnel(String name) {
        super(name);
    }

    /**
     * @return port that has corresponding AllowedIncomingTunnel object
     */
    public int getPortOnRemoteMachine() {
        return m_portOnRemoteMachine;
    }

    public void setPortOnRemoteMachine(int portOnRemoteMachine) {
        m_portOnRemoteMachine = portOnRemoteMachine;
    }

    public String getRemoteMachineAddress() {
        return m_remoteMachineAddress;
    }

    public void setRemoteMachineAddress(String remoteMachineAddress) {
        m_remoteMachineAddress = remoteMachineAddress;
    }

    public AllowedIncomingTunnel getIncomingTunnel() {
        return m_incomingTunnel;
    }

    public void setIncomingTunnel(AllowedIncomingTunnel incomingTunnel) {
        m_incomingTunnel = incomingTunnel;
    }
}
