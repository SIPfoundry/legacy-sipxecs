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

public class SipxProxyConfiguration extends SipxServiceConfiguration {

    private SipxService m_service;
    private SipxServiceManager m_sipxServiceManager;

    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }
    
    public void generate(SipxService service) {
        m_service = service;
    }

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(null);
        if (location != null) {
            context.put("location", location);
        }
        context.put("settings", m_service.getSettings().getSetting("proxy-configuration"));
        context.put("proxyService", m_service);

        SipxService callResolverService = m_sipxServiceManager.getServiceByBeanId(SipxCallResolverService.BEAN_ID);
        context.put("callResolverSettings", callResolverService.getSettings().getSetting("callresolver"));

        return context;
    }
}
