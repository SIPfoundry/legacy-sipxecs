/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.conference;

import static org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext.FEATURE;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.FieldMethodizer;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;
import org.springframework.beans.factory.annotation.Required;

public class ConferenceConfiguration implements ConfigProvider {
    private ConferenceBridgeContext m_conferenceBridgeContext;
    private VelocityEngine m_velocityEngine;
    private DomainManager m_domainManager;
    private String m_mohLocalStreamUrl;
    private String m_portAudioUrl;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(FEATURE, LocalizationContext.FEATURE)) {
            return;
        }

        Set<Location> locations = request.locations(manager);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE, location);
            ConfigUtils.enableCfengineClass(dir, "sipxconference.cfdat", enabled, "sipxconference");
            if (!enabled) {
                continue;
            }
            Writer wtr = new FileWriter(new File(dir, "conference.conf.xml"));
            try {
                String fqdn = location.getFqdn();
                Bridge bridge = m_conferenceBridgeContext.getBridgeByServer(fqdn);
                writeXml(wtr, location, m_domainManager.getDomain(), bridge);
            } catch (Exception e) {
                throw new IOException(e);
            } finally {
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    void writeXml(Writer wtr, Location location, Domain domain, Bridge bridge) throws IOException {
        VelocityContext context = new VelocityContext();
        context.put("domain", domain);
        // FieldMethodizer is used to allow for easy access to static fields (string constants)
        context.put("Bridge", new FieldMethodizer(Bridge.class.getName()));
        context.put("Conference", new FieldMethodizer(Conference.class.getName()));
        context.put("mohLocalStreamUrl", m_mohLocalStreamUrl);
        context.put("portAudioUrl", m_portAudioUrl);

        context.put("bridge", bridge);
        if (bridge != null) {
            Set<Conference> conferences = bridge.getConferences();

            context.put("conferences", conferences);
        }
        try {
            m_velocityEngine.mergeTemplate("sipxconference/conference.conf.xml.vm", context, wtr);
        } catch (Exception e) {
            throw new IOException(e);
        }
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    @Required
    public void setConferenceBridgeContext(ConferenceBridgeContext conferenceBridgeContext) {
        m_conferenceBridgeContext = conferenceBridgeContext;
    }

    @Required
    public void setMohLocalStreamUrl(String mohLocalStreamUrl) {
        m_mohLocalStreamUrl = mohLocalStreamUrl;
    }

    @Required
    public void setPortAudioUrl(String portAudioUrl) {
        m_portAudioUrl = portAudioUrl;
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }
}
