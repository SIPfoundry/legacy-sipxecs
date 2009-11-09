/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class SipxRecordingConfiguration extends SipxServiceConfiguration {
    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        SipxService service = getService(SipxRecordingService.BEAN_ID);
        context.put("service", service);
        return context;
    }

    @Override
    public boolean isReplicable(Location location) {
        return getSipxServiceManager().isServiceInstalled(location.getId(), SipxRecordingService.BEAN_ID);
    }
}
