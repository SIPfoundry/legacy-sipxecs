/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.address;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Feature;

/**
 * Sometime's the address you get depending on who's asking. You do not always have to supply a
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
