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
import java.util.List;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.springframework.beans.factory.annotation.Required;

public class TunnelConfiguration implements ConfigProvider, DaoEventListener {
    private VelocityEngine m_velocityEngine;
    private TunnelManager m_tunnelManager;
    private ConfigManager m_configManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(TunnelManager.FEATURE)) {
            return;
        }
        Set<Location> locations = request.locations(manager);
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(TunnelManager.FEATURE);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            ConfigUtils.enableCfengineClass(dir, "sipxtunnel.cfdat", "sipxtunneld", enabled);
            if (enabled) {
                List<AllowedIncomingTunnel> in = getIncomingTunnels(location, locations);
                Writer inConfig = new FileWriter(new File(dir, "tunnel-client.conf.cfdat"));
                try {
                    writeIncomingTunnels(inConfig, in);
                } finally {
                    IOUtils.closeQuietly(inConfig);
                }

                List<RemoteOutgoingTunnel> out = getOutgoingTunnels(location, locations);
                Writer outConfig = new FileWriter(new File(dir, "tunnel-server.conf.cfdat"));
                try {
                    writeOutgoingTunnels(outConfig, out);
                } finally {
                    IOUtils.closeQuietly(inConfig);
                }
            }
        }
    }

    void writeIncomingTunnels(Writer wtr, List<AllowedIncomingTunnel> tunnels) throws IOException {
        writeTunnels(wtr, "tunnel/tunnel-server.conf.vm", tunnels);
    }

    void writeOutgoingTunnels(Writer wtr, List<RemoteOutgoingTunnel> tunnels) throws IOException {
        writeTunnels(wtr, "tunnel/tunnel-client.conf.vm", tunnels);
    }

    List<RemoteOutgoingTunnel> getOutgoingTunnels(Location location, Set<Location> locations) {
        List<RemoteOutgoingTunnel> tunnels = new ArrayList<RemoteOutgoingTunnel>();
        List<Location> otherLocations = getOtherLocations(location, locations);
        for (TunnelProvider p : m_tunnelManager.getTunnelProviders()) {
            tunnels.addAll(p.getClientSideTunnels(otherLocations, location));
        }
        return tunnels;
    }

    void writeTunnels(Writer writer, String template, List< ? extends AbstractTunnel> tunnels) throws IOException {
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
    List<AllowedIncomingTunnel> getIncomingTunnels(Location location, Set<Location> locations) {
        List<AllowedIncomingTunnel> tunnels = new ArrayList<AllowedIncomingTunnel>();
        List<Location> otherLocations = getOtherLocations(location, locations);
        for (TunnelProvider p : m_tunnelManager.getTunnelProviders()) {
            tunnels.addAll(p.getServerSideTunnels(otherLocations, location));
        }
        return tunnels;
    }

    List<Location> getOtherLocations(Location location, Set<Location> locations) {
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
}
