/*
 *
 *
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.freeswitch.config;

import java.io.IOException;
import java.io.Writer;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchSettings;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.springframework.beans.factory.annotation.Required;


public class VarsConfiguration extends AbstractFreeswitchConfiguration {
    private FeatureManager m_featureManager;

    @Override
    protected String getTemplate() {
        return "freeswitch/vars.xml.vm";
    }

    @Override
    public void write(Writer writer, Location location, FreeswitchSettings settings) throws IOException {
        VelocityContext context = new VelocityContext();
        context.put("freeswitchDomain", getFsDomain(location));
        write(writer, context);
    }

    private String getFsDomain(Location location) {
        if (m_featureManager.isFeatureEnabled(Ivr.FEATURE, location)) {
            return "vm." + location.getFqdn();
        }
        return "$${local_ip_v4}";
    }

    @Override
    protected String getFileName() {
        return "vars.xml";
    }

    @Required
    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }
}
