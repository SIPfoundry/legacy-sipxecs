/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class PhoneLogConfiguration extends SipxServiceConfiguration {

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        SipxService service = getService(SipxConfigService.BEAN_ID);
        context.put("settings", service.getSettings().getSetting("configserver-config"));
        return context;
    }

    @Override
    public boolean isRestartRequired() {
        // prevent ConfigServer restart when "phonelog-config" file is replicated
        return false;
    }
}
