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

import org.sipfoundry.sipxconfig.service.SipxCallResolverService;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxService;

public abstract class EditCallResolverService extends EditSipxService {
    @SuppressWarnings("hiding")
    public static final String PAGE = "service/EditCallResolverService";

    @Override
    public String getMyBeanId() {
        return SipxCallResolverService.BEAN_ID;
    }

    /*
     * Override apply method to manually replicate proxy service config that depends on
     * CALLRESOLVER_CALL_STATE_DB setting from call resolver service
     */
    @Override
    public void apply() {
        super.apply();
        SipxService proxyService = getSipxServiceManager().getServiceByBeanId(SipxProxyService.BEAN_ID);
        getServiceConfigurator().replicateServiceConfig(proxyService);
    }
}
