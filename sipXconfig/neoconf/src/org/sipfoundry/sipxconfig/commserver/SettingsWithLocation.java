/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.commserver;

import org.sipfoundry.sipxconfig.setting.BeanWithGroups;

public abstract class SettingsWithLocation extends BeanWithGroups {
    private Location m_location;

    public abstract String getBeanId();

    /**
     * Should not be called. Only used to comply w/hibernate requirement of setter.
     * @param ignore
     */
    public void setBeanId(String ignore) {
    }

    public Location getLocation() {
        return m_location;
    }

    public void setLocation(Location location) {
        m_location = location;
    }
}
