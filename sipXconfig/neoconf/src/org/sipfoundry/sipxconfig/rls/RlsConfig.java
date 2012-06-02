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
package org.sipfoundry.sipxconfig.rls;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.springframework.beans.factory.annotation.Required;

public class RlsConfig implements ConfigProvider {
    private Rls m_rls;
    private ResourceLists m_lists;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(Rls.FEATURE)) {
            return;
        }

        Set<Location> locations = request.locations(manager);

        RlsSettings settings = m_rls.getSettings();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(Rls.FEATURE, location);
            ConfigUtils.enableCfengineClass(dir, "sipxrls.cfdat", enabled, "sipxrls");
            if (!enabled) {
                continue;
            }

            Writer wtr = new FileWriter(new File(dir, "sipxrls-config.part"));
            try {
                write(wtr, settings, location, manager.getDomainManager().getDomain());
            } finally {
                IOUtils.closeQuietly(wtr);
            }

            FileWriter xmlwtr = new FileWriter(new File(dir, "resource-lists.xml"));
            XmlFile listsXml = new XmlFile(xmlwtr);
            listsXml.write(m_lists.getDocument());
            IOUtils.closeQuietly(xmlwtr);
        }
    }

    void write(Writer wtr, RlsSettings settings, Location location, Domain domain) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.colonSeparated(wtr);
        config.writeSettings(settings.getSettings().getSetting("rls-config"));
        config.write("SIP_RLS_BIND_IP", location.getAddress());
        config.write("SIP_RLS_DOMAIN_NAME", domain.getName());
        config.write("SIP_RLS_AUTHENTICATE_REALM", domain.getSipRealm());
    }

    @Required
    public void setRls(Rls rls) {
        m_rls = rls;
    }

    @Required
    public void setRlsLists(ResourceLists lists) {
        m_lists = lists;
    }
}
