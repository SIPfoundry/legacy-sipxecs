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
package org.sipfoundry.sipxconfig.saa;

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
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.Location;

public class SaaConfiguration implements ConfigProvider {
    private VelocityEngine m_velocityEngine;
    private SaaManager m_saaManager;
    private CoreContext m_coreContext;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(SaaManager.FEATURE)) {
            return;
        }

        SaaSettings settings = m_saaManager.getSettings();
        String domainName = manager.getDomainManager().getDomainName();
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(SaaManager.FEATURE);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(SaaManager.FEATURE, location);
            ConfigUtils.enableCfengineClass(dir, "sipxsaa.cfdat", enabled, "sipxsaa");
            if (!enabled) {
                continue;
            }

            Writer saa = new FileWriter(new File(dir, "sipxsaa-config.part"));
            try {
                writeSaaConfig(saa, settings, location.getAddress());
            } finally {
                IOUtils.closeQuietly(saa);
            }

            Writer appear = new FileWriter(new File(dir, "appearance-groups.xml"));
            try {
                writeAppearance(appear, m_coreContext.getSharedUsers(), domainName);
            } finally {
                IOUtils.closeQuietly(appear);
            }
        }
    }

    void writeAppearance(Writer wtr, List<User> users, String domainName) throws IOException {
        VelocityContext context = new VelocityContext();
        context.put("sharedUsers", users);
        context.put("domainName", domainName);
        try {
            m_velocityEngine.mergeTemplate("sipxsaa/appearance-groups.vm", context, wtr);
        } catch (Exception e) {
            throw new IOException(e);
        }
    }

    void writeSaaConfig(Writer wtr, SaaSettings settings, String address)
        throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.colonSeparated(wtr);
        config.writeSettings(settings.getSettings().getSetting("saa-config"));
        config.write("SIP_SAA_BIND_IP", address);
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    public void setSaaManager(SaaManager saaManager) {
        m_saaManager = saaManager;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
