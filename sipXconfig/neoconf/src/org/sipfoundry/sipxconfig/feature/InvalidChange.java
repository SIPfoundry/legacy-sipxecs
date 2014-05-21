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

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;

public class InvalidChange {
    private static final String ERROR_REQUIRED_FEATURE_AT_LOCATION = "&error.requiredFeatureAtLocation.{0}.{1}.{2}";
    private Feature m_feature;
    private Location m_location;
    private InvalidChangeException m_message;
    private boolean m_allowAutoResolve = true;

    public InvalidChange(Feature f, InvalidChangeException msg) {
        m_feature = f;
        m_message = msg;
    }

    public InvalidChange(LocationFeature f, Location where, InvalidChangeException msg) {
        m_feature = f;
        m_location = where;
        m_message = msg;
    }

    public static InvalidChange requires(Feature subject, LocationFeature required, Location where) {
        InvalidChangeException msg = new InvalidChangeException(ERROR_REQUIRED_FEATURE_AT_LOCATION,
            subject, required, where.getHostname());
        return new InvalidChange(required, where, msg);
    }

    public static InvalidChange requires(Feature subject, LocationFeature required, List<Location> requiredLocations) {
        List<String> requiredHostnames = new ArrayList<String>();
        for (Location location : requiredLocations) {
            requiredHostnames.add(location.getHostname());
        }
        String errMessage = StringUtils.join(requiredHostnames.iterator(), ", ");
        InvalidChangeException msg = new InvalidChangeException(ERROR_REQUIRED_FEATURE_AT_LOCATION,
            subject, required, errMessage);
        return new InvalidChange(required, msg);
    }

    public static InvalidChange requires(Feature subject, Feature required) {
        InvalidChangeException msg = new InvalidChangeException("&error.requiredFeature.{0}.{1}", subject, required);
        return new InvalidChange(required, msg);
    }

    public Feature getFeature() {
        return m_feature;
    }

    public Location getLocation() {
        return m_location;
    }

    public UserException getMessage() {
        return m_message;
    }

    public boolean isAllowAutoResolve() {
        return m_allowAutoResolve;
    }

    public void setAllowAutoResolve(boolean allowAutoResolve) {
        m_allowAutoResolve = allowAutoResolve;
    }
}
