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

import java.util.Set;

import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.FieldMethodizer;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.SipxServiceConfiguration;
import org.springframework.beans.factory.annotation.Required;

public class ConferenceConfiguration extends SipxServiceConfiguration {
    private ConferenceBridgeContext m_conferenceBridgeContext;

    private DomainManager m_domainManager;
    private String m_mohLocalStreamUrl;
    private String m_portAudioUrl;

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

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        context.put("domain", m_domainManager.getDomain());
        // FieldMethodizer is used to allow for easy access to static fields (string constants)
        context.put("Bridge", new FieldMethodizer(Bridge.class.getName()));
        context.put("Conference", new FieldMethodizer(Conference.class.getName()));
        context.put("mohLocalStreamUrl", m_mohLocalStreamUrl);
        context.put("portAudioUrl", m_portAudioUrl);

        String fqdn = location.getFqdn();
        Bridge bridge = m_conferenceBridgeContext.getBridgeByServer(fqdn);
        context.put("bridge", bridge);
        if (bridge != null) {
            Set<Conference> conferences = bridge.getConferences();

            context.put("conferences", conferences);
        }

        return context;
    }
}
