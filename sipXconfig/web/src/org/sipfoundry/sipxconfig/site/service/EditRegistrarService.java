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
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.site.admin.commserver.Services;

public abstract class EditRegistrarService extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "service/EditRegistrarService";

    @InjectObject(value = "spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract SipxService getRegistrarService();

    public abstract void setRegistrarService(SipxService registrarService);

    public void pageBeginRender(PageEvent event) {
        if (getRegistrarService() == null) {
            SipxService proxyService = getSipxServiceManager().getServiceByBeanId(
                    SipxRegistrarService.BEAN_ID);
            setRegistrarService(proxyService);
        }
        
        if (getCallback() == null) {
            setCallback(new PageCallback(Services.PAGE));
        }
    }
    
    public void ok(IRequestCycle cycle) {
        apply();
        getCallback().performCallback(cycle);
    }

    public void apply() {
        getRegistrarService().validate();
        getSipxServiceManager().storeService(getRegistrarService());
    }

    public void cancel(IRequestCycle cycle) {
        getCallback().performCallback(cycle);
    }
}
