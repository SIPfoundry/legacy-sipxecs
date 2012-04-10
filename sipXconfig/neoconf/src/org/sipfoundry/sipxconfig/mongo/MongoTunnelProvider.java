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
package org.sipfoundry.sipxconfig.mongo;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.tunnel.AllowedIncomingTunnel;
import org.sipfoundry.sipxconfig.tunnel.RemoteOutgoingTunnel;
import org.sipfoundry.sipxconfig.tunnel.TunnelProvider;

/**
 * Mongo server does not have encrypted channel built in between servers so we need to build secure
 * tunnels between servers so it can replicate to other machines.
 */
public class MongoTunnelProvider implements TunnelProvider {
    private static final String MONGOD_PRIMARY = "mongod-primary";
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

        if (thisLocation == null) {
            return Collections.emptyList();
        }

        List<RemoteOutgoingTunnel> tunnels = new ArrayList<RemoteOutgoingTunnel>();
        if (thisLocation.isPrimary()) {
            tunnels.add(getAdditionalTunnel(MONGOD_PRIMARY, m_primaryPort, thisLocation.getAddress()));
        }

        Iterator<Location> otherLocationsIterator = otherLocations.iterator();
        for (int i = 0; otherLocationsIterator.hasNext();) {
            Location otherLocation = otherLocationsIterator.next();
            String name;
            int port;
            if (otherLocation.isPrimary()) {
                name = MONGOD_PRIMARY;
                port = m_primaryPort;
            } else {
                name = "mongod-" + i;
                port = m_primaryPort + (i + 1);
                i++;
            }
            tunnels.add(getAdditionalTunnel(name, port, otherLocation.getAddress()));
        }

        return tunnels;
    }

    private RemoteOutgoingTunnel getAdditionalTunnel(String name, int port, String address) {
        RemoteOutgoingTunnel additional = new RemoteOutgoingTunnel(name);
        additional.setLocalhostPort(port);
        additional.setPortOnRemoteMachine(m_incomingPort);
        additional.setRemoteMachineAddress(address);
        return additional;
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
