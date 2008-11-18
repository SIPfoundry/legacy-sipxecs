/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.service.freeswitch;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceConfiguration;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.beans.factory.annotation.Required;

import static org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType.MEDIA_SERVER;

public class SofiaConfiguration extends SipxServiceConfiguration {

    private DomainManager m_domainManager;

    private CoreContext m_coreContext;

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        context.put("domain", m_domainManager.getDomain());
        context.put("realm", m_domainManager.getAuthorizationRealm());

        User user = m_coreContext.getSpecialUser(MEDIA_SERVER);
        context.put("userMedia", user);

        SipxService service = getService(SipxFreeswitchService.BEAN_ID);
        Setting freeswitchConfig = service.getSettings().getSetting("freeswitch-config");
        context.put("settings", freeswitchConfig);

        return context;
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
