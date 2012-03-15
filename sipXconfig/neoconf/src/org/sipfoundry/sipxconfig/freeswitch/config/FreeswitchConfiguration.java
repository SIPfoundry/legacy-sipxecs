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
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchSettings;

public class FreeswitchConfiguration extends AbstractFreeswitchConfiguration {
    private DomainManager m_domainManager;

    @Override
    public void write(Writer writer, Location location, FreeswitchSettings settings) throws IOException {
        write(writer, m_domainManager.getDomain(), settings);
    }

    void write(Writer writer, Domain domain, FreeswitchSettings settings) throws IOException {
        VelocityContext context = new VelocityContext();
        context.put("dollar", "$");
        context.put("settings", settings.getSettings().getSetting("freeswitch-config"));
        context.put("domain", domain);
        write(writer, context);
    }

    @Override
    protected String getTemplate() {
        return "freeswitch/freeswitch.xml.vm";
    }

    @Override
    protected String getFileName() {
        return "freeswitch.xml";
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }
}
