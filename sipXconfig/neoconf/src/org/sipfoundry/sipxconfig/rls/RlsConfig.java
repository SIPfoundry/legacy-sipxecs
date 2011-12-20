/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.rls;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
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

        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(Rls.FEATURE);
        if (locations.isEmpty()) {
            return;
        }

        RlsSettings settings = m_rls.getSettings();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            Writer wtr = new FileWriter(new File(dir, "sipxrls-config.cfdat"));
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
        KeyValueConfiguration config = new KeyValueConfiguration(wtr);
        config.write(settings.getSettings().getSetting("rls-config"));
        config.write("SIP_RLS_BIND_IP", location.getAddress());
        config.write("SIP_RLS_DOMAIN_NAME", domain.getName());
        config.write("SIP_RLS_AUTHENTICATE_REALM", domain.getSipRealm());
    }

    @Required
    public void setRls(Rls rls) {
        m_rls = rls;
    }
}
