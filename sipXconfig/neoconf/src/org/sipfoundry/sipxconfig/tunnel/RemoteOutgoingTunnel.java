/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.tunnel;

/**
 * Used in stunnel client config, maps a local port to a remote port. Remote port should have a
 * corresponding AllowedIncomingTunnel config if this tunnel is to work
 */
public class RemoteOutgoingTunnel extends AbstractTunnel {
    private int m_portOnRemoteMachine;
    private String m_remoteMachineAddress;

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
}
