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

public class SipxProxyConfiguration extends SipxServiceConfiguration {

    private SipxService m_service;
    
    public void generate(SipxService service) {
        m_service = service;
    }

    @Override
    protected VelocityContext setupContext() {
        VelocityContext context = super.setupContext();
        context.put("settings", m_service.getSettings().getSetting("proxy-configuration"));
        context.put("proxyService", m_service);
        return context;
    }
}
