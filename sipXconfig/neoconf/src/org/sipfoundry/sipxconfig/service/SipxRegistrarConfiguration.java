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

public class SipxRegistrarConfiguration extends SipxServiceConfiguration {
    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        SipxService registrarService = getService(SipxRegistrarService.BEAN_ID);
        context.put("settings", registrarService.getSettings());
        context.put("registrarService", registrarService);
        context.put("proxyService", getService(SipxProxyService.BEAN_ID));
        context.put("parkService", getService(SipxParkService.BEAN_ID));

        return context;
    }

    @Override
    public boolean isReplicable(Location location) {
        return getSipxServiceManager().isServiceInstalled(location.getId(), SipxRegistrarService.BEAN_ID);
    }

}
