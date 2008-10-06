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

public class SipxRlsConfiguration extends SipxServiceConfiguration {

    private SipxService m_service;
    
    public void generate(SipxService service) {
        m_service = service;
    }

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(null);
        if (location != null) {
            context.put("location", location);
        }
        context.put("settings", m_service.getSettings().getSetting("rls-config"));
        context.put("rlsService", m_service);
        return context;
    }
}
