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

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.service.SipxRlsService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

public abstract class EditResourceListService extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "service/EditResourceListService";
    
    @InjectObject(value = "spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract SipxService getResourceListService();
    public abstract void setResourceListService(SipxService registrarService);
    
    public void pageBeginRender(PageEvent event) {
        if (getResourceListService() == null) {
            SipxService resourceListService = getSipxServiceManager().getServiceByBeanId(
                    SipxRlsService.BEAN_ID);
            setResourceListService(resourceListService);
        }
    }    

    public void apply() {
        getResourceListService().validate();
        getSipxServiceManager().storeService(getResourceListService());
    }
}
