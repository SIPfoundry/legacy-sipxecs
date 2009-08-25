/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.openfire;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceConfiguration;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;
import org.springframework.beans.factory.annotation.Required;

import static org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType.XMPP_SERVER;

public class SipxOpenfireConfiguration extends SipxServiceConfiguration {
    private CoreContext m_coreContext;

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        SipxService openfireService = getService(SipxOpenfireService.BEAN_ID);
        context.put("settings", openfireService.getSettings());
        context.put("service", openfireService);

        String username = XMPP_SERVER.getUserName();
        User user = m_coreContext.getSpecialUser(XMPP_SERVER);
        context.put("username", username);
        context.put("password", user.getSipPassword());
        context.put("resource-list", SpeedDial.getResourceListId(username, true));

        return context;
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
