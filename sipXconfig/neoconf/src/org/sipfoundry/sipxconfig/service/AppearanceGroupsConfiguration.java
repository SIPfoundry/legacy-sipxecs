/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.service;

import java.util.Collection;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

public class AppearanceGroupsConfiguration extends SipxServiceConfiguration {

    private CoreContext m_coreContext;

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        Collection<User> sharedUsers = m_coreContext.getSharedUsers();
        context.put("sharedUsers", sharedUsers);
        SipxService saaService = getService(SipxSaaService.BEAN_ID);
        context.put("saaService", saaService);

        return context;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
