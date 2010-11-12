/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.service.freeswitch;

import java.util.Set;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.acccode.AccCodeContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchExtensionCollector;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxServiceConfiguration;
import org.springframework.beans.factory.annotation.Required;

/**
 * Generates default_context.xml.in
 */
public class DefaultContextConfiguration extends SipxServiceConfiguration {

    private ConferenceBridgeContext m_conferenceContext;
    private AccCodeContext m_acccodeContext;
    private FreeswitchExtensionCollector m_freeswitchExtensionCollector;

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        Bridge bridge = m_conferenceContext.getBridgeByServer(location.getFqdn());
        if (bridge != null) {
            Set conferences = bridge.getConferences();
            context.put("conferences", conferences);
        }
        if (m_acccodeContext.isEnabled()) {
            boolean acccodeActive = true;
            context.put("acccode", acccodeActive);
        }
        context.put("freeswitchExtensions", m_freeswitchExtensionCollector.getExtensions());
        return context;
    }

    @Required
    public void setConferenceContext(ConferenceBridgeContext conferenceContext) {
        m_conferenceContext = conferenceContext;
    }

    @Required
    public void setAccCodeContext(AccCodeContext accCodeContext) {
        m_acccodeContext = accCodeContext;
    }

    @Required
    public void setFreeswitchExtensionCollector(FreeswitchExtensionCollector collector) {
        m_freeswitchExtensionCollector = collector;
    }

    @Override
    public boolean isReplicable(Location location) {
        return getSipxServiceManager().isServiceInstalled(location.getId(), SipxFreeswitchService.BEAN_ID);
    }
}
