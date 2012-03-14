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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class EditLocationPage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "admin/commserver/EditLocationPage";
    private static final String CONFIG_TAB = "configureLocation";
    private static final String SERVICES_TAB = "listServices";

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Integer getLocationId();

    public abstract void setLocationId(Integer locationId);

    public abstract Location getLocationBean();

    public abstract void setLocationBean(Location location);

    public abstract boolean isRegistered();

    public abstract void setRegistered(boolean registered);

    @Persist
    @InitialValue("literal:configureLocation")
    public abstract String getTab();

    public abstract void setTab(String tab);

    public Collection<String> getAvailableTabNames() {
        Collection<String> tabNames = new ArrayList<String>();
        tabNames.addAll(Arrays.asList(CONFIG_TAB, SERVICES_TAB));
        return tabNames;
    }

    public void pageBeginRender(PageEvent event) {
        Location location = getLocationBean();
        if (location != null) {
            // make sure we have correct bean ID persisted
            if (!location.isNew()) {
                setLocationId(location.getId());
            }
            return;
        }
        if (getLocationId() != null) {
            location = getLocationsManager().getLocation(getLocationId());
            setRegistered(location.isRegistered());
        } else {
            location = new Location();
            setTab(CONFIG_TAB);
        }
        setLocationBean(location);
    }

    @Override
    public String getBreadCrumbTitle() {
        return null == getLocationId() ? "&crumb.new.server"
            : getLocationsManager().getLocation(getLocationId()).getFqdn();
    }
}
