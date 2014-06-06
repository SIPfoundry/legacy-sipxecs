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

    protected String getFsDomain(Location location) {
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

    @Required
    public FeatureManager getFeatureManager() {
        return m_featureManager;
    }
}
