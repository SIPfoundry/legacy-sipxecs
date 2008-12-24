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

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class SipxCallResolverConfiguration extends SipxServiceConfiguration {
    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        SipxService service = getService(SipxCallResolverService.BEAN_ID);
        context.put("settings", service.getSettings().getSetting("callresolver"));
        context.put("callresolverService", service);
        return context;
    }

    @Override
    public boolean isReplicable(Location location) {
        return getSipxServiceManager().isServiceInstalled(location.getId(), SipxCallResolverService.BEAN_ID);
    }
}
