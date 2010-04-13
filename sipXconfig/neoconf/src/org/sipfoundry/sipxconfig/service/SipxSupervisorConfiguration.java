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

public class SipxSupervisorConfiguration extends SipxServiceConfiguration {

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        SipxSupervisorService service = (SipxSupervisorService) getSipxServiceManager().
            getServiceByBeanId(SipxSupervisorService.BEAN_ID);
        context.put("supervisorHost", location.getFqdn());
        context.put("supervisorLogLevel", service.getLogLevel());
        return context;
    }

    @Override
    public boolean isReplicable(Location location) {
        return true;
    }


}
