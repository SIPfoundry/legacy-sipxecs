/**
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
package org.sipfoundry.sipxconfig.feature;

import static java.lang.String.format;

import org.sipfoundry.sipxconfig.commserver.Location;

public class InvalidChange {
    private Feature m_feature;
    private Location m_location;
    private String m_message;

    public InvalidChange(Feature f, String msg) {
        m_feature = f;
        m_message = msg;
    }

    public InvalidChange(LocationFeature f, Location where, String msg) {
        m_feature = f;
        m_location = where;
        m_message = msg;
    }

    public static InvalidChange requires(Feature subject, LocationFeature required, Location where) {
        String msg = format("Feature %s requires feature %s be enabled %s", subject, required, where.getHostname());
        return new InvalidChange(required, msg);
    }

    public static InvalidChange requires(Feature subject, Feature required) {
        String msg = format("Feature %s requires feature %s", subject, required);
        return new InvalidChange(required, msg);
    }

    public Feature getFeature() {
        return m_feature;
    }

    public Location getLocation() {
        return m_location;
    }

    public String getMessage() {
        return m_message;
    }
}
