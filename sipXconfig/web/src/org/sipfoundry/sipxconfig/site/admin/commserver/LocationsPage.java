/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.apache.tapestry.IAsset;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.InjectState;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.commserver.DnsGenerator;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.site.UserSession;
import org.sipfoundry.sipxconfig.site.common.BreadCrumb;

public abstract class LocationsPage extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "admin/commserver/LocationsPage";

    @InjectState(value = "userSession")
    public abstract UserSession getUserSession();

    @InjectObject("spring:serviceConfigurator")
    public abstract ServiceConfigurator getServiceConfigurator();

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectObject("spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    @InjectObject("spring:domainManager")
    public abstract DomainManager getDomainManager();

    @InjectObject("spring:dnsGenerator")
    public abstract DnsGenerator getDnsGenerator();

    @InjectObject("spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectPage(EditLocationPage.PAGE)
    public abstract EditLocationPage getEditLocationPage();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Bean
    public abstract SelectMap getSelections();

    @Asset("/images/breadcrumb_separator.png")
    public abstract IAsset getBreadcrumbSeparator();

    @Asset("/images/server.png")
    public abstract IAsset getServerIcon();

    public abstract Location getCurrentRow();

    public abstract Collection<Location> getLocations();

    public abstract void setLocations(Collection<Location> locations);

    public abstract Collection<Integer> getRowsToDelete();

    public Collection getAllSelected() {
        return getSelections().getAllSelected();
    }

    public void pageBeginRender(PageEvent event) {
        if (getLocations() == null) {
            setLocations(Arrays.asList(getLocationsManager().getLocations()));
        }
    }

    public IPage editLocation(int locationId) {
        EditLocationPage editLocationPage = getEditLocationPage();
        editLocationPage.setLocationId(locationId);
        editLocationPage.setReturnPage(this, getBreadCrumbs());
        return editLocationPage;
    }

    public IPage addLocation() {
        EditLocationPage editLocationPage = getEditLocationPage();
        editLocationPage.setLocationId(null);
        editLocationPage.setReturnPage(this, getBreadCrumbs());
        return editLocationPage;
    }

    public void deleteLocations() {
        Collection<Integer> selectedLocations = getSelections().getAllSelected();
        for (Integer id : selectedLocations) {
            Location locationToDelete = getLocationsManager().getLocation(id);
            try {
                getLocationsManager().deleteLocation(locationToDelete);
            } catch (UserException e) {
                getValidator().record(e, getMessages());
            }
        }

        // update locations list
        setLocations(null);
    }

    public void generateProfiles() {
        Collection<Integer> selectedLocations = getSelections().getAllSelected();
        if (!selectedLocations.isEmpty()) {
            // HACK: push dataSets and files that are not the part of normal service replication
            getServiceConfigurator().initLocations();
        }
        for (Integer id : selectedLocations) {
            Location locationToActivate = getLocationsManager().getLocation(id);
            if (!locationToActivate.isRegistered()) {
                continue;
            }

            getServiceConfigurator().replicateLocation(locationToActivate);
            getServiceConfigurator().enforceRole(locationToActivate);

            if (locationToActivate.isPrimary()) {
                getDnsGenerator().generate();
            }
        }
    }

    public List<BreadCrumb> getBreadCrumbs() {
        List<BreadCrumb> breadCrumbs = new ArrayList<BreadCrumb>();
        breadCrumbs.add(new BreadCrumb(getPageName(), "&title", getMessages()));
        return breadCrumbs;
    }
}
