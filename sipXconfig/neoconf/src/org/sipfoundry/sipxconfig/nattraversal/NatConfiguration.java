/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.nattraversal;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.sbc.SbcManager;
import org.sipfoundry.sipxconfig.sbc.SbcRoutes;

public class NatConfiguration implements ConfigProvider {
    private VelocityEngine m_velocityEngine;
    private NatTraversal m_nat;
    private SbcManager m_sbcManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(NatTraversal.FEATURE)) {
            return;
        }
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(
                ProxyManager.FEATURE);
        if (locations == null || locations.isEmpty()) {
            return;
        }

        NatSettings settings = m_nat.getSettings();
        int proxyTcpPort = manager.getAddressManager().getSingleAddress(ProxyManager.TCP_ADDRESS).getPort();
        int proxyTlsPort = manager.getAddressManager().getSingleAddress(ProxyManager.TLS_ADDRESS).getPort();
        SbcRoutes routes = m_sbcManager.getRoutes();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            Writer writer = new FileWriter(new File(dir, "nattraversalrules.xml"));
            try {
                write(writer, settings, location, routes, proxyTcpPort, proxyTlsPort);
            } finally {
                IOUtils.closeQuietly(writer);
            }
        }
    }

    void write(Writer writer, NatSettings settings, Location location, SbcRoutes routes, int proxyTcpPort,
            int proxyTlsPort) throws IOException {
        VelocityContext context = new VelocityContext();
        context.put("settings", settings);
        context.put("location", location);
        context.put("xmlRpcPort", settings.getXmlRpcPort());
        context.put("proxyTcpPort", proxyTcpPort);
        context.put("proxyTlsPort", proxyTlsPort);
        context.put("routes", routes);
        try {
            m_velocityEngine.mergeTemplate("nattraversal/nattraversalrules.vm", context, writer);
        } catch (Exception e) {
            throw new IOException(e);
        }
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    public void setNat(NatTraversal nat) {
        m_nat = nat;
    }
}
