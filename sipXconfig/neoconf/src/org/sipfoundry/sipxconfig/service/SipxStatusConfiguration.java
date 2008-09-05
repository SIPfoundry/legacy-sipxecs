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

public class SipxStatusConfiguration extends SipxServiceConfiguration {

    private SipxService m_service;

    @Override
    public void generate(SipxService service) {
        m_service = service;
    }
    
    @Override
    protected VelocityContext setupContext() {
        VelocityContext context = super.setupContext();
        context.put("settings", m_service.getSettings().getSetting("status-config"));
        context.put("statusService", m_service);
        return context;
    }
}
