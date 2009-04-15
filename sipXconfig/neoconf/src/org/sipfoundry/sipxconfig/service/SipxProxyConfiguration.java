/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class SipxProxyConfiguration extends SipxServiceConfiguration {
    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);

        SipxService service = getService(SipxProxyService.BEAN_ID);
        context.put("settings", service.getSettings().getSetting("proxy-configuration"));
        context.put("proxyService", service);

        SipxService callResolverService = getService(SipxCallResolverService.BEAN_ID);
        context.put("callResolverSettings", callResolverService.getSettings().getSetting("callresolver"));

        return context;
    }

    @Override
    public boolean isReplicable(Location location) {
        return getSipxServiceManager().isServiceInstalled(location.getId(), SipxProxyService.BEAN_ID);
    }
}
