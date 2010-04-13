/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.cdr;

import java.util.Arrays;
import java.util.Collection;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.service.SipxCallResolverService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class CdrPage extends UserBasePage {
    private static final String ACTIVE_TAB = "active";
    private static final String HISTORIC_TAB = "historic";
    private static final String REPORTS_TAB = "reports";

    @Override
    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    @InitialValue(value = "literal:active")
    public abstract String getTab();

    public abstract void setTab(String tab);

    @InjectObject("spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    public abstract boolean isCallResolverInstalled();

    public abstract void setCallResolverInstalled(boolean callResolverInstalled);

    public Collection<String> getTabNames() {
        if (isCallResolverInstalled()) {
            return Arrays.asList(ACTIVE_TAB, HISTORIC_TAB, REPORTS_TAB);
        }
        return Arrays.asList(HISTORIC_TAB, REPORTS_TAB);
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        Location primaryLocation = getLocationsManager().getPrimaryLocation();
        setCallResolverInstalled(getSipxServiceManager().isServiceInstalled(primaryLocation.getId(),
                SipxCallResolverService.BEAN_ID));
        if (!isCallResolverInstalled() && getTab().equals(ACTIVE_TAB)) {
            setTab(HISTORIC_TAB);
        }
    }
}
