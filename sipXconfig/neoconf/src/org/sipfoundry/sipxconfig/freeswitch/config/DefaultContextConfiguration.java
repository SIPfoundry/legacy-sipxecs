/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.freeswitch.config;

import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.acccode.AuthCodes;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchExtension;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchExtensionCollector;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchSettings;
import org.springframework.beans.factory.annotation.Required;

/**
 * Generates default_context.xml.in
 */
public class DefaultContextConfiguration extends AbstractFreeswitchConfiguration {
    private ConferenceBridgeContext m_conferenceContext;
    private FreeswitchExtensionCollector m_freeswitchExtensionCollector;
    private FeatureManager m_featureManager;

    @Override
    public void write(Writer writer, Location location, FreeswitchSettings settings) throws IOException {
        Bridge bridge = m_conferenceContext.getBridgeByServer(location.getFqdn());
        boolean authCodes = m_featureManager.isFeatureEnabled(AuthCodes.FEATURE, location);
        List<FreeswitchExtension> extensions = m_freeswitchExtensionCollector.getExtensions();
        write(writer, location, bridge, authCodes, extensions);
    }

    void write(Writer writer, Location location, Bridge bridge, boolean authCodes, List<FreeswitchExtension> extensions)
        throws IOException {
        VelocityContext context = new VelocityContext();
        if (bridge != null) {
            Set<Conference> conferences = bridge.getConferences();
            context.put("conferences", conferences);
        }
        if (authCodes) {
            context.put("acccode", true);
        }
        List<FreeswitchExtension> freeswitchExtensions = new ArrayList<FreeswitchExtension>();
        for (FreeswitchExtension extension : extensions) {
            if (extension.isEnabled()) {
                freeswitchExtensions.add(extension);
            }
        }
        context.put("location", location);
        context.put("dollar", "$");
        context.put("freeswitchExtensions", freeswitchExtensions);
        write(writer, context);
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
        return "dialplan/sipX_context.xml";
    }

    @Override
    protected String getTemplate() {
        return "freeswitch/default_context.xml.vm";
    }

    @Required
    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }
}
