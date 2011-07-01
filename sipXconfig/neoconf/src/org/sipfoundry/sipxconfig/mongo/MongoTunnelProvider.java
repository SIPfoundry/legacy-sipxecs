/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mongo;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.tunnel.AllowedIncomingTunnel;
import org.sipfoundry.sipxconfig.tunnel.RemoteOutgoingTunnel;
import org.sipfoundry.sipxconfig.tunnel.TunnelProvider;

/**
 * Mongo server does not have encrypted channel built in between servers so we need to build secure
 * tunnels between servers so it can replicate to other machines.
 */
public class MongoTunnelProvider implements TunnelProvider {
    private int m_localPort = 27017;
    private int m_tunnelPortToMaster = 27018;
    private int m_tunnelStartingPortToSecondary = 27019;

    /**
     * @return The port number mongod runs on
     */
    public int getLocalPort() {
        return m_localPort;
    }

    public void setLocalPort(int localPort) {
        m_localPort = localPort;
    }

    /**
     * @return The local port that will lead to the mongo master server
     */
    public int getTunnelPortToMaster() {
        return m_tunnelPortToMaster;
    }

    public void setTunnelPortToMaster(int tunnelPortToMaster) {
        m_tunnelPortToMaster = tunnelPortToMaster;
    }

    /**
     * @return The local port that will lead to the first in a possible list of mongo server that are
     * *not* the master.  Port numbers will increase by one the number of non-master servers you have
     */
    public int getTunnelStartingPortToSecondary() {
        return m_tunnelStartingPortToSecondary;
    }

    public void setTunnelStartingPortToSecondary(int tunnelStartingPortToSecondary) {
        m_tunnelStartingPortToSecondary = tunnelStartingPortToSecondary;
    }

    @Override
    public Collection<RemoteOutgoingTunnel> getClientSideTunnels(Collection<Location> otherLocations,
            Location thisLocation) {

        if (otherLocations.size() == 0 || thisLocation == null) {
            return Collections.emptyList();
        }

        List<RemoteOutgoingTunnel> tunnels = new ArrayList<RemoteOutgoingTunnel>();
        RemoteOutgoingTunnel primary = new RemoteOutgoingTunnel("mongod-primary");
        primary.setLocalhostPort(m_tunnelStartingPortToSecondary);
        primary.setPortOnRemoteMachine(m_tunnelPortToMaster);
        tunnels.add(primary);

        for (int i = 0; i < otherLocations.size(); i++) {
            RemoteOutgoingTunnel additional = new RemoteOutgoingTunnel("mongod-" + i);
            additional.setLocalhostPort(m_tunnelStartingPortToSecondary + i + 1);
            additional.setPortOnRemoteMachine(m_tunnelPortToMaster);
            tunnels.add(additional);
        }

        return tunnels;
    }

    @Override
    public Collection<AllowedIncomingTunnel> getServerSideTunnels(Collection<Location> otherLocations,
            Location thisLocation) {

        if (otherLocations.size() == 0 || thisLocation == null) {
            return Collections.emptyList();
        }

        List<AllowedIncomingTunnel> tunnels = new ArrayList<AllowedIncomingTunnel>();
        AllowedIncomingTunnel primary = new AllowedIncomingTunnel("mongod");
        primary.setLocalhostPort(m_tunnelPortToMaster);
        primary.setAllowedConnectionsPort(m_localPort);
        tunnels.add(primary);
        return tunnels;
    }
}
