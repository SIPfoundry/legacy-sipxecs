/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.site.admin;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

public abstract class CallRateLimitPage extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "admin/CallRateLimitPage";

    @InjectObject("spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    @InjectObject("spring:serviceConfigurator")
    public abstract ServiceConfigurator getServiceConfigurator();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract SipxService getService();

    public abstract void setService(SipxService service);

    @Override
    public void pageBeginRender(PageEvent event) {
        SipxService sipxService = getSipxServiceManager().getServiceByBeanId(SipxProxyService.BEAN_ID);
        setService(sipxService);
    }

    public void saveService() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        SipxService service = getService();
        service.validate();
        getSipxServiceManager().storeService(service);
        getServiceConfigurator().replicateServiceConfig(service);
    }

}
