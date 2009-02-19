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
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

import static org.sipfoundry.sipxconfig.components.TapestryUtils.getMessage;

public abstract class EditSipxService extends PageWithCallback implements PageBeginRenderListener {

    @InjectObject("spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    @InjectObject("spring:serviceConfigurator")
    public abstract ServiceConfigurator getServiceConfigurator();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract SipxService getSipxService();

    public abstract void setSipxService(SipxService service);

    protected abstract String getBeanId();

    public void pageBeginRender(PageEvent event) {
        if (getSipxService() == null) {
            SipxService sipxService = getSipxServiceManager().getServiceByBeanId(
                    getBeanId());
            setSipxService(sipxService);
        }
    }

    public String getTitle() {
        String serviceBeanId = getSipxService().getBeanId();
        return getMessage(getMessages(), "label." + serviceBeanId, serviceBeanId);
    }

    public void apply() {
        SipxService service = getSipxService();
        service.validate();
        getSipxServiceManager().storeService(service);
        getServiceConfigurator().replicateServiceConfig(service);
    }
}
