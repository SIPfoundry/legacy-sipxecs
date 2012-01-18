/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.restserver;

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
import org.sipfoundry.sipxconfig.domain.Domain;

public class RestConfiguration implements ConfigProvider {
    private RestServer m_restServer;
    private VelocityEngine m_velocityEngine;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!manager.getFeatureManager().isFeatureEnabled(RestServer.FEATURE)) {
            return;
        }

        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(RestServer.FEATURE);
        if (locations.isEmpty()) {
            return;
        }

        RestServerSettings settings = m_restServer.getSettings();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            Writer wtr = new FileWriter(new File(dir, "sipxrest-config.xml"));
            try {
                write(wtr, settings, location, manager.getDomainManager().getDomain());
            } finally {
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    void write(Writer wtr, RestServerSettings settings, Location location, Domain domain) throws IOException {
        VelocityContext context = new VelocityContext();
        context.put("settings", settings.getSettings().getSetting("rest-config"));
        context.put("location", location);
        context.put("domainName", domain.getName());
        try {
            m_velocityEngine.mergeTemplate("sipxrest/sipxrest-config.vm", context, wtr);
        } catch (Exception e) {
            throw new IOException(e);
        }
    }

    public void setRestServer(RestServer restServer) {
        m_restServer = restServer;
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }
}
