/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.service;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

public abstract class EditProxyService extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "service/EditProxyService";

    @InjectObject(value = "spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract SipxService getProxyService();

    public abstract void setProxyService(SipxService proxyService);

    public void pageBeginRender(PageEvent event) {
        if (getProxyService() == null) {
            SipxService proxyService = getSipxServiceManager().getServiceByBeanId(
                    SipxProxyService.BEAN_ID);
            setProxyService(proxyService);
        }
    }
    
    public void ok(IRequestCycle cycle) {
        apply();
        getCallback().performCallback(cycle);
    }

    public void apply() {
        getSipxServiceManager().storeService(getProxyService());
    }

    public void cancel(IRequestCycle cycle) {
        getCallback().performCallback(cycle);
    }
}
