/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
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
