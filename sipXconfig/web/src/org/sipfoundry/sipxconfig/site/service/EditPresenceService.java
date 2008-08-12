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
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.service.SipxPresenceService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

public abstract class EditPresenceService extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "service/EditPresenceService";

    @InjectObject(value = "spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();
    
    @InjectObject(value = "spring:sipxReplicationContext")
    public abstract SipxReplicationContext getSipxReplicationContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract SipxService getPresenceService();

    public abstract void setPresenceService(SipxService presenceService);

    public void pageBeginRender(PageEvent event) {
        if (getPresenceService() == null) {
            SipxService proxyService = getSipxServiceManager().getServiceByBeanId(
                    SipxPresenceService.BEAN_ID);
            setPresenceService(proxyService);
        }
    }
    
    public void ok(IRequestCycle cycle) {
        apply();
        getCallback().performCallback(cycle);
    }

    public void apply() {
        getSipxServiceManager().storeService(getPresenceService());
        getSipxReplicationContext().generate(DataSet.ALIAS);
    }

    public void cancel(IRequestCycle cycle) {
        getCallback().performCallback(cycle);
    }
}
