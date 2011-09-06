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
import java.util.Iterator;
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
    private int m_incomingPort = 27018;
    private int m_primaryPort = 27019;

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
        return m_incomingPort;
    }

    public void setTunnelPortToMaster(int incomingPort) {
        m_incomingPort = incomingPort;
    }

    /**
     * @return The local port that will lead to the first in a possible list of mongo server that are
     * *not* the master.  Port numbers will increase by one the number of non-master servers you have
     */
    public int getPrimaryPort() {
        return m_primaryPort;
    }

    public void setPrimaryPort(int primaryPort) {
        m_primaryPort = primaryPort;
    }

    @Override
    public Collection<RemoteOutgoingTunnel> getClientSideTunnels(Collection<Location> otherLocations,
            Location thisLocation) {

        if (otherLocations.size() == 0 || thisLocation == null) {
            return Collections.emptyList();
        }

        List<RemoteOutgoingTunnel> tunnels = new ArrayList<RemoteOutgoingTunnel>();
        Iterator<Location> otherLocationsIterator = otherLocations.iterator();
        for (int i = 0; otherLocationsIterator.hasNext();) {
            Location otherLocation = otherLocationsIterator.next();
            String name;
            int port;
            if (otherLocation.isPrimary()) {
                name = "mongod-primary";
                port = m_primaryPort;
            } else {
                name = "mongod-" + i;
                port = m_primaryPort + (i + 1);
                i++;
            }
            RemoteOutgoingTunnel additional = new RemoteOutgoingTunnel(name);
            additional.setLocalhostPort(port);
            additional.setPortOnRemoteMachine(m_incomingPort);
            additional.setRemoteMachineAddress(otherLocation.getAddress());
            tunnels.add(additional);
        }

        return tunnels;
    }

    @Override
    public Collection<AllowedIncomingTunnel> getServerSideTunnels(Collection<Location> otherLocations,
            Location thisLocation) {

        if (thisLocation == null) {
            return Collections.emptyList();
        }

        List<AllowedIncomingTunnel> tunnels = new ArrayList<AllowedIncomingTunnel>();
        AllowedIncomingTunnel primary = new AllowedIncomingTunnel("mongod");
        primary.setLocalhostPort(m_localPort);
        primary.setAllowedConnectionsPort(m_incomingPort);
        tunnels.add(primary);
        return tunnels;
    }
}
