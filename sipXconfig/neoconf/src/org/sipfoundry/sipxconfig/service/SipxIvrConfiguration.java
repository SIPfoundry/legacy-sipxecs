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

public class SipxIvrConfiguration extends SipxServiceConfiguration {
    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        context.put("service", getService(SipxIvrService.BEAN_ID));
        context.put("statusService", getService(SipxStatusService.BEAN_ID));
        context.put("restService", getService(SipxRestService.BEAN_ID));
        context.put("imbotService", getService(SipxImbotService.BEAN_ID));
        context.put("sipxServiceManager", getSipxServiceManager());
        return context;
    }

    @Override
    public boolean isReplicable(Location location) {
        return getSipxServiceManager().isServiceInstalled(location.getId(), SipxIvrService.BEAN_ID);
    }
}
