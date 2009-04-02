/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import java.util.Arrays;
import java.util.Collection;

import org.apache.tapestry.IAsset;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

public abstract class LocationsPage extends BasePage implements PageBeginRenderListener {
    public static final String PAGE = "admin/commserver/LocationsPage";

    @InjectObject("spring:serviceConfigurator")
    public abstract ServiceConfigurator getServiceConfigurator();

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectObject("spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    @InjectObject("spring:domainManager")
    public abstract DomainManager getDomainManager();

    @InjectPage(EditLocationPage.PAGE)
    public abstract EditLocationPage getEditLocationPage();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Bean
    public abstract SelectMap getSelections();

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
        editLocationPage.setReturnPage(this);
        return editLocationPage;
    }

    public IPage addLocation() {
        EditLocationPage editLocationPage = getEditLocationPage();
        editLocationPage.setLocationId(null);
        editLocationPage.setReturnPage(this);
        return editLocationPage;
    }

    public void deleteLocations() {
        Collection<Integer> selectedLocations = getSelections().getAllSelected();
        for (Integer id : selectedLocations) {
            Location locationToDelete = getLocationsManager().getLocation(id);
            getLocationsManager().deleteLocation(locationToDelete);
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
        }
    }
}
