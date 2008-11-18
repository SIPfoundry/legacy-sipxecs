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

import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.SelectMap;

public abstract class LocationsPage extends BasePage implements PageBeginRenderListener {
    public static final String PAGE = "admin/commserver/LocationsPage";

    @InjectObject(value = "spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectPage(value = EditLocationPage.PAGE)
    public abstract EditLocationPage getEditLocationPage();

    @InjectPage(value = AddLocationPage.PAGE)
    public abstract AddLocationPage getAddLocationPage();

    @Bean
    public abstract SelectMap getSelections();

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
        editLocationPage.setLocationId(new Integer(locationId));
        editLocationPage.setCallback(new PageCallback(LocationsPage.PAGE));
        return editLocationPage;
    }

    public IPage addLocation() {
        AddLocationPage addLocationPage = getAddLocationPage();
        addLocationPage.setReturnPage(getPage());
        addLocationPage.setCallback(new PageCallback(LocationsPage.PAGE));
        return addLocationPage;
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
}
