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


import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.callback.ICallback;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class LocationPanel extends BaseComponent {
    private boolean m_advanced;

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @Parameter
    public abstract ICallback getCallback();

    @Parameter(required = true)
    public abstract Location getLocationBean();

    public void saveLocation() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        Location location = getLocationBean();
        getLocationsManager().saveLocation(location);
    }

    public boolean isAdvanced() {
        return m_advanced;
    }

    public void setAdvanced(boolean advanced) {
        m_advanced = advanced;
    }
}
