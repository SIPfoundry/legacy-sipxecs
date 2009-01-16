/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import java.util.Arrays;
import java.util.Collection;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class EditLocationPage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "admin/commserver/EditLocationPage";

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectObject("spring:sipxProcessContext")
    public abstract SipxProcessContext getSipxProcessContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Integer getLocationId();

    public abstract void setLocationId(Integer locationId);

    public abstract void setLocationBean(Location location);

    public abstract Location getLocationBean();

    public abstract Collection<String> getAvailableTabNames();

    public abstract void setAvailableTabNames(Collection<String> tabNames);

    @Persist
    @InitialValue("literal:listServices")
    public abstract String getTab();

    public void pageBeginRender(PageEvent event) {
        if (getLocationId() != null) {
            Location location = getLocationsManager().getLocation(getLocationId());
            setLocationBean(location);
        }

        setAvailableTabNames(Arrays.asList("configureLocation", "listServices"));
    }

    public void saveLocation() {
        if (TapestryUtils.isValid(this)) {
            Location location = getLocationBean();
            getLocationsManager().storeLocation(location);
            getSipxProcessContext().enforceRole(location);
        }
    }
}
