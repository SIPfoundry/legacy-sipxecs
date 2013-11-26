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
package org.sipfoundry.sipxconfig.restserver;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingUtil;

public class RestConfiguration implements ConfigProvider {
    private RestServer m_restServer;
    private VelocityEngine m_velocityEngine;
    private String m_restSettingKey = "rest-config";

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        Set<Location> locations = request.locations(manager);
        if (locations.isEmpty()) {
            return;
        }

        RestServerSettings settings = m_restServer.getSettings();
        Address sipxcdrApi = manager.getAddressManager().getSingleAddress(AdminContext.SIPXCDR_DB_ADDRESS);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(RestServer.FEATURE, location);
            ConfigUtils.enableCfengineClass(dir, "sipxrest.cfdat", enabled, "sipxrest");
            if (!enabled) {
                continue;
            }

            Setting restSettings = settings.getSettings().getSetting(m_restSettingKey);
            String log4jFileName = "log4j-rest.properties.part";
            SettingUtil.writeLog4jSetting(restSettings, dir, log4jFileName);

            Writer wtr = new FileWriter(new File(dir, "sipxrest-config.xml"));
            try {
                write(wtr, settings, location, manager.getDomainManager().getDomain(), sipxcdrApi);
            } finally {
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    void write(Writer wtr, RestServerSettings settings, Location location,
            Domain domain, Address sipxcdrApi) throws IOException {
        VelocityContext context = new VelocityContext();
        context.put("settings", settings.getSettings().getSetting(m_restSettingKey));
        context.put("location", location);
        context.put("domainName", domain.getName());
        context.put("sipxcdrDbAddress", sipxcdrApi.toString());
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
