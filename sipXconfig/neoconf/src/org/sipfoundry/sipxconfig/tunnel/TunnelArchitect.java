/**
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

import static java.lang.String.format;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallRule;

/**
 * Design a series of tunnels from server to server so that none of the ports collide and each
 * server can export their service and import services from others
 */
public class TunnelArchitect {
    private AddressManager m_addressManager;
    private Map<Location, List<AllowedIncomingTunnel>> m_server;
    private List<RemoteOutgoingTunnel> m_client;
    private int m_serverStartPort;
    private int m_clientStartPort;

    void build(Collection<Location> locations, Collection<DefaultFirewallRule> rules) {
        m_server = new HashMap<Location, List<AllowedIncomingTunnel>>();
        m_client = new ArrayList<RemoteOutgoingTunnel>();
        int clientPort = m_clientStartPort;
        for (Location location : locations) {
            int serverPort = m_serverStartPort;
            List<AllowedIncomingTunnel> server = new ArrayList<AllowedIncomingTunnel>();
            m_server.put(location, server);
            for (DefaultFirewallRule rule : rules) {
                if (FirewallRule.SystemId.CLUSTER != rule.getSystemId()) {
                    continue;
                }
                List<Address> addresses = m_addressManager.getAddresses(rule.getAddressType(), location);
                for (Address address : addresses) {
                    if (!address.getAddress().equals(location.getAddress())) {
                        continue;
                    }

                    String name = tunnelName(location, rule.getAddressType());
                    AllowedIncomingTunnel s = new AllowedIncomingTunnel(name);
                    s.setLocalhostPort(address.getCanonicalPort());
                    s.setAllowedConnectionsPort(serverPort);
                    s.setProtocol(address.getType().getProtocol());
                    server.add(s);

                    RemoteOutgoingTunnel c = new RemoteOutgoingTunnel(name);
                    c.setIncomingTunnel(s);
                    c.setLocalhostPort(clientPort);
                    c.setPortOnRemoteMachine(serverPort);
                    c.setRemoteMachineAddress(address.getAddress());
                    c.setProtocol(address.getType().getProtocol());
                    m_client.add(c);

                    s.setOutgoingTunnel(c);

                    serverPort++;
                    clientPort++;
                }
            }
        }
    }

    public Collection <AllowedIncomingTunnel> getAllowedIncomingTunnels(Location l) {
        return m_server.get(l);
    }

    @SuppressWarnings("unchecked")
    public Collection <RemoteOutgoingTunnel> getRemoteOutgoingTunnels(final Location l) {
        Predicate remoteOnly = new Predicate() {
            @Override
            public boolean evaluate(Object o) {
                return !((RemoteOutgoingTunnel) o).getRemoteMachineAddress().equals(l.getAddress());
            }
        };
        return CollectionUtils.select(m_client, remoteOnly);
    }

    /**
     * @return any useful, arbitrary, unique name
     */
    String tunnelName(Location l, AddressType t) {
        return format("%s-%d", t.getId(), l.getId());
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    public int getServerStartPort() {
        return m_serverStartPort;
    }

    public void setServerStartPort(int serverStartPort) {
        m_serverStartPort = serverStartPort;
    }

    public int getClientStartPort() {
        return m_clientStartPort;
    }

    public void setClientStartPort(int clientStartPort) {
        m_clientStartPort = clientStartPort;
    }
}
