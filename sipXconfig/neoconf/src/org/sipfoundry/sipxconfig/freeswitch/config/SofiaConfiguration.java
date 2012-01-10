/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.freeswitch.config;

import static org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType.MEDIA_SERVER;

import java.io.IOException;
import java.io.Writer;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchSettings;
import org.springframework.beans.factory.annotation.Required;

public class SofiaConfiguration extends AbstractFreeswitchConfiguration {
    private DomainManager m_domainManager;
    private CoreContext m_coreContext;

    @Override
    protected String getTemplate() {
        return "freeswitch/sofia.conf.xml.vm";
    }

    @Override
    protected String getFileName() {
        return "sip_profiles/sipX_profile.xml";
    }

    @Override
    public void write(Writer writer, Location location, FreeswitchSettings settings) throws IOException {
        User userMedia = m_coreContext.getSpecialUser(MEDIA_SERVER);
        write(writer, settings, m_domainManager.getDomain(), userMedia);
    }

    void write(Writer writer, FreeswitchSettings settings, Domain domain, User userMedia) throws IOException {
        VelocityContext context = new VelocityContext();
        context.put("domain", domain);
        context.put("realm", domain.getSipRealm());
        context.put("userMedia", userMedia);
        context.put("settings", settings.getSettings().getSetting("freeswitch-config"));
        write(writer, context);
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }
}
