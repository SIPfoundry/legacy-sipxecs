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
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;

public class NatConfiguration implements ConfigProvider {
    private VelocityEngine m_velocityEngine;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (request.applies(NatTraversal.FEATURE)) {
            List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(ProxyManager.FEATURE);
            for (Location location : locations) {
                File dir = manager.getLocationDataDirectory(location);
                VelocityContext context = new VelocityContext();
                FileWriter writer = new FileWriter(new File(dir, "nattraversalrules.xml"));
                try {
                    m_velocityEngine.mergeTemplate("nattraversal/nattraversalrules.vm", context, writer);
                } catch (Exception e) {
                    throw new IOException(e);
                }
                IOUtils.closeQuietly(writer);
            }
        }
    }

}
