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

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class EditLocationPage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "admin/commserver/EditLocationPage";
    
    @InjectObject(value = "spring:locationsManager")
    public abstract LocationsManager getLocationsManager();
    
    @Bean
    public abstract SipxValidationDelegate getValidator();
    
    @Persist
    public abstract Integer getLocationId();
    public abstract void setLocationId(Integer locationId);
    
    public abstract void setLocationBean(Location location);
    public abstract Location getLocationBean();
    
    public void pageBeginRender(PageEvent event) {
        if (getLocationBean() == null) {
            if (getLocationId() != null) {
                Location location = getLocationsManager().getLocation(getLocationId());
                setLocationBean(location);
            } else {
                setLocationBean(new Location());
            }
        }
    }
    
    public void saveLocation() {
        if (TapestryUtils.isValid(this)) {
            getLocationsManager().storeLocation(getLocationBean());
        }
    }
}
