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
package org.sipfoundry.sipxconfig.commserver;

import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.systemaudit.ConfigChangeType;
import org.sipfoundry.sipxconfig.systemaudit.SystemAuditable;

public abstract class SettingsWithLocation extends BeanWithGroups implements SystemAuditable {
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

    public String getEntityIdentifier() {
        return getClass().getName();
    }

    @Override
    public String getConfigChangeType() {
        return ConfigChangeType.SETTINGS.getName();
    }
}
