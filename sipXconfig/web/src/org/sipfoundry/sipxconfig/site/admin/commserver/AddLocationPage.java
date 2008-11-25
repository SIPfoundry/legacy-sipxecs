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

import org.apache.commons.lang.RandomStringUtils;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class AddLocationPage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "admin/commserver/AddLocationPage";

    private static final int LOCATION_PASSWORD_LEN = 8;

    @InjectObject(value = "spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract Location getLocationBean();

    public abstract void setLocationBean(Location location);

    public void pageBeginRender(PageEvent event) {
        if (getLocationBean() == null) {
            Location location = new Location();
            location.setPassword(RandomStringUtils.randomAlphanumeric(LOCATION_PASSWORD_LEN));
            setLocationBean(location);
        }
    }

    public void saveLocation() {
        if (TapestryUtils.isValid(this)) {
            getLocationsManager().storeLocation(getLocationBean());
        }
    }
}
