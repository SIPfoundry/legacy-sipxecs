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
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.monitoring.MonitoringContext;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

public abstract class EditLocationPage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "admin/commserver/EditLocationPage";
    public static final String MONITOR_TAB = "monitorTarget";
    private static final String CONFIG_TAB = "configureLocation";
    private static final String NAT_TAB = "natLocation";
    private static final String BUNDLE_TAB = "configureBundle";

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectObject("spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    @InjectObject(value = "spring:monitoringContext")
    public abstract MonitoringContext getMonitoringContext();

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
    @InitialValue("literal:listServices")
    public abstract String getTab();

    public abstract void setTab(String tab);

    public Collection<String> getAvailableTabNames() {
        Collection<String> tabNames = new ArrayList<String>();
        tabNames.addAll(Arrays.asList(CONFIG_TAB, BUNDLE_TAB, "listServices", NAT_TAB));
        if (getMonitoringContext().isEnabled()) {
            tabNames.add(MONITOR_TAB);
        }
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
            if (!location.isRegistered()) {
                setTab(CONFIG_TAB);
            }
        } else {
            location = new Location();
            location.initBundles(getSipxServiceManager());
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
