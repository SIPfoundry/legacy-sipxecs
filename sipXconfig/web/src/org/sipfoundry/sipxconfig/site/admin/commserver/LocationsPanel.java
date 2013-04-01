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
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.InjectState;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.logging.AuditLogContext;
import org.sipfoundry.sipxconfig.site.UserSession;
import org.sipfoundry.sipxconfig.site.common.BreadCrumb;

public abstract class LocationsPanel extends BaseComponent implements PageBeginRenderListener {
    @InjectState(value = "userSession")
    public abstract UserSession getUserSession();

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectObject("spring:domainManager")
    public abstract DomainManager getDomainManager();

    @InjectObject("spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectObject("spring:configManager")
    public abstract ConfigManager getConfigManager();

    @InjectObject("spring:auditLogContext")
    public abstract AuditLogContext getAuditLogContext();

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

    public abstract Set<Location> getRegisteredLocations();

    public abstract void setRegisteredLocations(Set<Location> locations);

    public abstract void setLocations(Collection<Location> locations);

    public abstract Collection<Integer> getRowsToDelete();

    public abstract void setDetails(String details);

    public abstract String getDetails();

    @InjectPage(LocationStatePage.PAGE)
    public abstract LocationStatePage getLocationStatePage();

    public String getStatusLabel() {
        Location currentLocation = getCurrentRow();
        boolean registered = getRegisteredLocations().contains(currentLocation);
        String key = registered ? "CONFIGURED" : "UNINITIALIZED";
        return getMessages().getMessage(key);
    }

    public Collection getAllSelected() {
        return getSelections().getAllSelected();
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        if (getLocations() == null) {
            setLocations(Arrays.asList(getLocationsManager().getLocations()));
        }
        if (getRegisteredLocations() == null) {
            Collection<Location> reg = getConfigManager().getRegisteredLocations(getLocations());
            setRegisteredLocations(new HashSet<Location>(reg));
        }
    }

    public IPage seeState(Location location) {
        LocationStatePage page = getLocationStatePage();
        page.setServer(location);
        page.setReturnPage(getPage());
        return page;
    }

    public IPage editLocation(int locationId) {
        EditLocationPage editLocationPage = getEditLocationPage();
        editLocationPage.setLocationId(locationId);
        editLocationPage.setReturnPage(getPage(), getBreadCrumbs());
        return editLocationPage;
    }

    public IPage addLocation() {
        EditLocationPage editLocationPage = getEditLocationPage();
        editLocationPage.setLocationId(null);
        editLocationPage.setReturnPage(getPage(), getBreadCrumbs());
        return editLocationPage;
    }

    public void deleteLocations() {
        Collection<Integer> selectedLocations = getSelections()
                .getAllSelected();
        for (Integer id : selectedLocations) {
            Location locationToDelete = getLocationsManager().getLocation(id);
            try {
                // calling deleteSomething will trigger interceptor and publish on delete events
                // and we don't want this for primary server
                if (locationToDelete.isPrimary()) {
                    getValidator().record(new ValidatorException(getMessages().format("error.delete.primary",
                            locationToDelete.getFqdn())));
                } else {
                    getLocationsManager().deleteLocation(locationToDelete);
                }
            } catch (UserException e) {
                getValidator().record(e, getMessages());
            }
        }

        // update locations list
        setLocations(null);
    }
    
    public void resetKeys() {
        getConfigManager().resetKeys(getSelectedLocations());    	
    }
    
    public void generateProfiles() {
        getConfigManager().sendProfiles(getSelectedLocations());
    }

    Collection<Location> getSelectedLocations() {
        Collection<Integer> selectedIds = getSelections().getAllSelected();
        Collection<Location> selectedLocations = new ArrayList<Location>();
        for (Location location : getLocations()) {
            if (selectedIds.contains(location.getId())) {
                selectedLocations.add(location);
            }
        }	
        return selectedLocations;    	
    }

    public boolean isFailedState() {
        return !getCurrentRow().isUninitialized() && getCurrentRow().isInConfigurationErrorState();
    }

    public List<BreadCrumb> getBreadCrumbs() {
        List<BreadCrumb> breadCrumbs = new ArrayList<BreadCrumb>();
        breadCrumbs.add(new BreadCrumb(getPage().getPageName(), "&title", getMessages()));
        return breadCrumbs;
    }
}
