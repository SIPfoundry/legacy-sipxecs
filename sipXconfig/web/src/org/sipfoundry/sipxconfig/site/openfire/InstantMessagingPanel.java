/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.openfire;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.setting.Setting;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class InstantMessagingPanel extends BaseComponent implements PageBeginRenderListener {
    private static final String SIPX_OPENFIRE_BEAN_ID = "sipxOpenfireService";

    @InjectObject("spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    @InjectObject("spring:serviceConfigurator")
    public abstract ServiceConfigurator getServiceConfigurator();

    @InjectObject("spring:coreContext")
    public abstract CoreContext getCoreContext();

    @Parameter(required = true)
    public abstract String getSettingGroup();

    public abstract SipxService getSipxService();

    public abstract void setSipxService(SipxService sipxService);

    public void pageBeginRender(PageEvent event) {
        if (getSipxService() == null) {
            setSipxService(getSipxServiceManager().getServiceByBeanId(SIPX_OPENFIRE_BEAN_ID));
        }
    }

    public void apply() {
        if (TapestryUtils.isValid(this)) {
            SipxService service = getSipxService();
            service.validate();
            getSipxServiceManager().storeService(service);
            getServiceConfigurator().replicateServiceConfig(service);
        }
    }

    public Setting getSettings() {
        return getSipxService().getSettings().getSetting(getSettingGroup());
    }
}
