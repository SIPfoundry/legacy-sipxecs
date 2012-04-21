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

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Collection;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.YamlConfiguration;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;

public class TunnelConfiguration implements ConfigProvider, DaoEventListener {
    private ConfigManager m_configManager;
    private FirewallManager m_firewallManager;
    private LocationsManager m_locationsManager;
    private AddressManager m_addressManager;
    private TunnelManager m_tunnelManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(TunnelManager.FEATURE)) {
            return;
        }
        Set<Location> locations = request.locations(manager);
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(TunnelManager.FEATURE);
        TunnelSettings settings = m_tunnelManager.getSettings();
        TunnelArchitect architect = new TunnelArchitect();
        if (enabled) {
            architect.setAddressManager(m_addressManager);
            architect.setServerStartPort(settings.getServerStartPort());
            architect.setClientStartPort(settings.getClientStartPort());
            architect.build(m_locationsManager.getLocationsList(), m_firewallManager.getDefaultFirewallRules());
        }
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            ConfigUtils.enableCfengineClass(dir, "tunnel.cfdat", enabled, "tunnel");
            if (!enabled) {
                continue;
            }
            Collection<AllowedIncomingTunnel> in = architect.getAllowedIncomingTunnels(location);
            Collection<RemoteOutgoingTunnel> out = architect.getRemoteOutgoingTunnels(location);
            Writer w = new FileWriter(new File(dir, "tunnel.yaml"));
            try {
                writeConfig(w, in, out);
            } finally {
                IOUtils.closeQuietly(w);
            }
        }
    }

    private void writeConfig(Writer w, Collection<AllowedIncomingTunnel> in, Collection<RemoteOutgoingTunnel> out)
        throws IOException {
        YamlConfiguration c = new YamlConfiguration(w);
        c.startArray("in");
        String name = ":name";
        String localPort = ":local_port";
        for (AllowedIncomingTunnel i : in) {
            c.write(name, i.getName());
            c.write(localPort, i.getLocalhostPort());
            c.write(":incoming_port", i.getAllowedConnectionsPort());
            c.nextElement();
        }
        c.endArray();
        c.startArray("out");
        for (RemoteOutgoingTunnel o : out) {
            c.write(name, o.getName());
            c.write(localPort, o.getLocalhostPort());
            c.write(":remote_port", o.getPortOnRemoteMachine());
            c.write(":remote_address", o.getRemoteMachineAddress());
            c.nextElement();
        }
        c.endArray();
    }

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof Location) {
            m_configManager.configureEverywhere(TunnelManager.FEATURE);
        }
    }

    @Override
    public void onSave(Object entity) {
        if (entity instanceof Location) {
            Location l = (Location) entity;
            if (l.hasFqdnOrIpChangedOnSave()) {
                m_configManager.configureEverywhere(TunnelManager.FEATURE);
            }
        }
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public void setFirewallManager(FirewallManager firewallManager) {
        m_firewallManager = firewallManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    public void setTunnelManager(TunnelManager tunnelManager) {
        m_tunnelManager = tunnelManager;
    }
}
