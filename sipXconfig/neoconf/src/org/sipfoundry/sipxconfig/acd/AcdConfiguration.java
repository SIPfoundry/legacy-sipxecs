/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.service.SipxAcdService;
import org.sipfoundry.sipxconfig.service.SipxServiceConfiguration;

public class AcdConfiguration extends SipxServiceConfiguration {

    private AcdContext m_acdContext;

    @Override
    protected VelocityContext setupContext(Location location) {
        AcdServer server = m_acdContext.getAcdServerForLocationId(location.getId());
        int acdPort = server.getPort();

        VelocityContext context = super.setupContext(location);
        context.put("serverPort", acdPort);

        return context;
    }

    public void setAcdContext(AcdContext acdContext) {
        m_acdContext = acdContext;
    }

    @Override
    public boolean isReplicable(Location location) {
        return getSipxServiceManager().isServiceInstalled(location.getId(), SipxAcdService.BEAN_ID);
    }
}
