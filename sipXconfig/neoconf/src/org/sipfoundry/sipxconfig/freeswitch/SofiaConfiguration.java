/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.freeswitch;

import static org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType.MEDIA_SERVER;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.annotation.Required;

public class SofiaConfiguration extends FreeswitchConfigFile {
    private DomainManager m_domainManager;
    private CoreContext m_coreContext;

    @Override
    protected String getFileName() {
        return "freeswitch/sofia.conf.xml";
    }

    @Override
    protected void setupContext(VelocityContext context, Location location, FreeswitchSettings settings) {
        context.put("domain", m_domainManager.getDomain());
        context.put("realm", m_domainManager.getAuthorizationRealm());
        User user = m_coreContext.getSpecialUser(MEDIA_SERVER);
        context.put("userMedia", user);
        context.put("settings", settings.getSettings().getSetting("freeswitch-config"));
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
