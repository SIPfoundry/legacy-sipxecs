/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.api.model;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.sipfoundry.sipxconfig.commserver.Location;

@XmlRootElement(name = "Servers")
public class ServerList {

    private List<ServerBean> m_servers;

    public void setServers(List<ServerBean> servers) {
        m_servers = servers;
    }

    @XmlElement(name = "Server")
    public List<ServerBean> getServers() {
        if (m_servers == null) {
            m_servers = new ArrayList<ServerBean>();
        }
        return m_servers;
    }

    public static ServerList convertLocationList(List<Location> locations, Collection<Location> registeredLocations) {
        List<ServerBean> servers = new ArrayList<ServerBean>();
        for (Location location : locations) {
            servers.add(ServerBean.convertLocation(location, registeredLocations.contains(location)));
        }
        ServerList list = new ServerList();
        list.setServers(servers);
        return list;
    }
}
