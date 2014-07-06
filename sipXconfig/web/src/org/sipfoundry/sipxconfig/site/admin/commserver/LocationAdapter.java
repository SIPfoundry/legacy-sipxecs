/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.site.admin.commserver;

import org.apache.tapestry.IActionListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;

@SuppressWarnings("rawtypes")
public abstract class LocationAdapter implements OptionAdapter, IActionListener {

    private Integer m_id;

    private Location m_location;

    public LocationAdapter(Location location) {
        m_location = location;
    }

    public Integer getId() {
        return m_id;
    }

    public void setId(Integer id) {
        m_id = id;
    }

    public Location getSelectedLocation() {
        return m_location;
    }

    public void setSelectedLocation(Location location) {
        m_location = location;
    }

    public Object getValue(Object option, int index) {
        return this;
    }

    public String getLabel(Object option, int index) {
        return m_location.getFqdn();
    }

    public String squeezeOption(Object option, int index) {
        return m_location.getId().toString();
    }

    public String getMethodName() {
        return null;
    }
}
