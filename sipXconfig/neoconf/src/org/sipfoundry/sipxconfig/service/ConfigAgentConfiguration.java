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

public class ConfigAgentConfiguration extends SipxServiceConfiguration {

    @Override
    public boolean isReplicable(Location location) {
        return true;
    }

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        SipxService service = getService(SipxConfigAgentService.BEAN_ID);
        context.put("settings", service.getSettings().getSetting("configagent-config"));
        return context;
    }
}
