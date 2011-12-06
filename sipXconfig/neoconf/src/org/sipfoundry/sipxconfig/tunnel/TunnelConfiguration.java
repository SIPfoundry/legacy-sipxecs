/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.tunnel;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.springframework.beans.factory.annotation.Required;

public class TunnelConfiguration implements ConfigProvider {
    private VelocityEngine m_velocityEngine;
    private TunnelManager m_tunnelManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (request.applies(TunnelManager.FEATURE)) {
            List<Location> locations = Arrays.asList(manager.getLocationManager().getLocations());
            for (Location location : locations) {
                File dir = manager.getLocationDataDirectory(location);
                List<AllowedIncomingTunnel> in = getIncomingTunnels(location, locations);
                write(dir, in, "tunnel/tunnel-client.conf.vm", "tunnel/tunnel-client.conf.cfdat");
                List<RemoteOutgoingTunnel> out = getOutgoingTunnels(location, locations);
                write(dir, out, "tunnel/tunnel-server.conf.vm", "tunnel/tunnel-server.conf.cfdat");
            }
        }
    }

    void write(File dir, List<? extends AbstractTunnel> tunnels, String template, String dst) throws IOException {
        FileWriter writer = null;
        try {
            File file = new File(dir, dst);
            writer = new FileWriter(file);
            writeTunnels(writer, template, tunnels);
        } finally {
            IOUtils.closeQuietly(writer);
        }
    }

    List<RemoteOutgoingTunnel> getOutgoingTunnels(Location location, List<Location> locations) {
        List<RemoteOutgoingTunnel> tunnels = new ArrayList<RemoteOutgoingTunnel>();
        List<Location> otherLocations = getOtherLocations(location, locations);
        for (TunnelProvider p : m_tunnelManager.getTunnelProviders()) {
            tunnels.addAll(p.getClientSideTunnels(otherLocations, location));
        }
        return tunnels;
    }

    void writeTunnels(Writer writer, String template, List<? extends AbstractTunnel> tunnels) throws IOException {
        VelocityContext context = new VelocityContext();
        context.put("tunnelManager", m_tunnelManager);
        context.put("tunnels", tunnels);
        try {
            m_velocityEngine.mergeTemplate(template, context, writer);
        } catch (Exception e) {
            throw new IOException(e);
        }
    }

    /**
     * Collect all the server-side tunnel configs from all the providers
     */
    List<AllowedIncomingTunnel> getIncomingTunnels(Location location, List<Location> locations) {
        List<AllowedIncomingTunnel> tunnels = new ArrayList<AllowedIncomingTunnel>();
        List<Location> otherLocations = getOtherLocations(location, locations);
        for (TunnelProvider p : m_tunnelManager.getTunnelProviders()) {
            tunnels.addAll(p.getServerSideTunnels(otherLocations, location));
        }
        return tunnels;
    }

    List<Location> getOtherLocations(Location location, List<Location> locations) {
        // Although it should be only locations that have stunnel service running, if the
        // system is unregistered, service will not be running, but we'll want to generate
        // the configuration for it. Also, providers and ultimately control what the actual
        // tunnels are anyway
        List<Location> otherLocations = new ArrayList<Location>(locations);
        otherLocations.remove(location);
        return otherLocations;
    }

    @Required
    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    @Required
    public void setTunnelManager(TunnelManager tunnelManager) {
        m_tunnelManager = tunnelManager;
    }
}
