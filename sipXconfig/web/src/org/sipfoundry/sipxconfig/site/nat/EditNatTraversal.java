/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.site.nat;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.site.common.BreadCrumb;

public abstract class EditNatTraversal extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "nat/EditNatTraversal";

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectPage(EditNatLocation.PAGE)
    public abstract EditNatLocation getEditNatLocation();

    @Persist
    @InitialValue(value = "literal:locations")
    public abstract String getTab();

    public abstract Collection<Location> getLocations();

    public abstract void setLocations(Collection<Location> locations);

    public abstract Location getCurrentRow();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Override
    public void pageBeginRender(PageEvent arg0) {
        if (getLocations() == null) {
            setLocations(Arrays.asList(getLocationsManager().getLocations()));
        }
    }

    public IPage editNat(int locationId) {
        EditNatLocation editLocationPage = getEditNatLocation();
        editLocationPage.setLocationId(locationId);
        editLocationPage.setReturnPage(this, getBreadCrumbs());
        return editLocationPage;
    }

    public List<BreadCrumb> getBreadCrumbs() {
        List<BreadCrumb> breadCrumbs = new ArrayList<BreadCrumb>();
        breadCrumbs.add(new BreadCrumb(getPageName(), "&title", getMessages()));
        return breadCrumbs;
    }
}
