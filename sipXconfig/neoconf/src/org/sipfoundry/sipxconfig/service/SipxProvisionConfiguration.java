/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class SipxProvisionConfiguration extends SipxServiceConfiguration {

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        SipxService provisionService = getService(SipxProvisionService.BEAN_ID);
        context.put("settings", provisionService.getSettings().getSetting("provision-config"));
        context.put("provisionService", provisionService);
        context.put("locationsManager", provisionService.getLocationsManager());

        return context;
    }
}
