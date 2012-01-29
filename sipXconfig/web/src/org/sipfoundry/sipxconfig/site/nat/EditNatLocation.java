/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
