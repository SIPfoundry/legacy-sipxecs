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

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class EditNatLocation extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "nat/EditNatLocation";

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Integer getLocationId();

    public abstract void setLocationId(Integer locationId);

    public abstract Location getLocationBean();

    public abstract void setLocationBean(Location location);


    public void pageBeginRender(PageEvent event) {
        if (getLocationBean() == null) {
            if (getLocationId() != null) {
                setLocationBean(getLocationsManager().getLocation(getLocationId()));
            }
        }
    }

    @Override
    public String getBreadCrumbTitle() {
        return null == getLocationId() ? "&crumb.edit.nat"
            : getLocationsManager().getLocation(getLocationId()).getFqdn();
    }
}
