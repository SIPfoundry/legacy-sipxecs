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

public class VoicemailConfiguration extends SipxServiceConfiguration {

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        SipxService mediaService = getService(SipxMediaService.BEAN_ID);
        SipxService statusService = getService(SipxStatusService.BEAN_ID);
        context.put("settings", mediaService .getSettings().getSetting("mediaserver-config"));
        context.put("mediaService", mediaService);
        context.put("statusService", statusService);
        return context;
    }
}
