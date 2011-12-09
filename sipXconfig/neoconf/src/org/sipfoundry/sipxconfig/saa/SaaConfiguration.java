/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public class SaaConfiguration implements ConfigProvider {
    private VelocityEngine m_velocityEngine;
    private SaaManager m_saaManager;
    private CoreContext m_coreContext;
    private DomainManager m_domainManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(SaaManager.FEATURE)) {
            return;
        }

        SaaSettings settings = m_saaManager.getSettings();
        String domainName = m_domainManager.getDomainName();
        String realm = m_domainManager.getAuthorizationRealm();
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(SaaManager.FEATURE);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            Writer saa = new FileWriter(new File(dir, "sipxsaa.properties.cfdat"));
            try {
                writeSaaConfig(saa, settings, location.getAddress(), domainName, realm);
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

    void writeSaaConfig(Writer wtr, SaaSettings settings, String address, String domainName, String realm)
        throws IOException {
        KeyValueConfiguration config = new KeyValueConfiguration(wtr);
        config.write(settings.getSettings().getSetting("saa-config"));
        config.write("SIP_SAA_BIND_IP", address);
        config.write("SIP_SAA_DOMAIN_NAME", domainName);
        config.write("SIP_SAA_AUTHENTICATE_REALM", realm);
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    public void setSaaManager(SaaManager saaManager) {
        m_saaManager = saaManager;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }
}
