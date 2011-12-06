/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.freeswitch;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.acccode.AuthCodes;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.springframework.beans.factory.annotation.Required;

/**
 * Generates default_context.xml.in
 */
public class DefaultContextConfiguration extends FreeswitchConfigFile {
    private ConferenceBridgeContext m_conferenceContext;
    private FreeswitchExtensionCollector m_freeswitchExtensionCollector;
    private FeatureManager m_featureManager;

    @Override
    protected void setupContext(VelocityContext context, Location location, FreeswitchSettings settings) {
        Bridge bridge = m_conferenceContext.getBridgeByServer(location.getFqdn());
        if (bridge != null) {
            Set conferences = bridge.getConferences();
            context.put("conferences", conferences);
        }
        if (m_featureManager.isFeatureEnabled(AuthCodes.FEATURE, location)) {
            context.put("acccode", true);
        }
        List<FreeswitchExtension> freeswitchExtensions = new ArrayList<FreeswitchExtension>();
        for (FreeswitchExtension extension : m_freeswitchExtensionCollector.getExtensions()) {
            if (extension.isEnabled()) {
                freeswitchExtensions.add(extension);
            }
        }
        context.put("freeswitchExtensions", freeswitchExtensions);
    }

    @Required
    public void setConferenceContext(ConferenceBridgeContext conferenceContext) {
        m_conferenceContext = conferenceContext;
    }

    @Required
    public void setFreeswitchExtensionCollector(FreeswitchExtensionCollector collector) {
        m_freeswitchExtensionCollector = collector;
    }

    @Override
    protected String getFileName() {
        return "freeswitch/default_context.conf.xml";
    }

    @Required
    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }
}
