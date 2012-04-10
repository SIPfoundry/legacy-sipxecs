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
package org.sipfoundry.sipxconfig.address;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Feature;

/**
 * Sometime's the address you get depends on who's asking. You do not always have to supply a
 * source and you should not always expect a source.
 */
public class AddressRequester {
    private Feature m_feature;
    private Location m_location;
    private AddressType m_type;

    public AddressRequester() {
    }

    public AddressRequester(Feature feature) {
        this(feature, null);
    }

    public AddressRequester(Location location) {
        this(null, location);
    }

    public AddressRequester(Feature feature, Location location) {
        m_feature = feature;
        m_location = location;
    }

    public Feature getFeature() {
        return m_feature;
    }

    public void setFeature(Feature feature) {
        m_feature = feature;
    }

    public Location getLocation() {
        return m_location;
    }

    public void setLocation(Location location) {
        m_location = location;
    }

    public AddressType getType() {
        return m_type;
    }

    public void setType(AddressType type) {
        m_type = type;
    }
}
