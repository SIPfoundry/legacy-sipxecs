/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.openfire;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.setting.Setting;


public abstract class ServerToServerPanel extends BaseComponent implements PageBeginRenderListener {
    private static final String SIPX_OPENFIRE_BEAN_ID = "sipxOpenfireService";

    @InjectObject("spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    @InjectObject("spring:serviceConfigurator")
    public abstract ServiceConfigurator getServiceConfigurator();

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    public abstract SipxService getSipxOpenfireService();

    public abstract void setSipxOpenfireService(SipxService sipxService);

    public void pageBeginRender(PageEvent event) {
        if (getSipxOpenfireService() == null) {
            setSipxOpenfireService(getSipxServiceManager().getServiceByBeanId(SIPX_OPENFIRE_BEAN_ID));
        }
    }
    public void apply() {
        if (TapestryUtils.isValid(this)) {
            SipxService sipxOpenfireService = getSipxOpenfireService();
            sipxOpenfireService.validate();
            getSipxServiceManager().storeService(sipxOpenfireService);
            getServiceConfigurator().replicateServiceConfig(sipxOpenfireService);
        }
    }
    public Setting getOpenfireSettings() {
        return getSipxOpenfireService().getSettings().getSetting("openfire-server-to-server");
    }

}
