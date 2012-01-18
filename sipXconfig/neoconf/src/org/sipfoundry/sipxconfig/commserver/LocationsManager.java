/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.commserver;

import java.util.List;

public interface LocationsManager {
    String CONTEXT_BEAN_NAME = "locationsManager";

    Location[] getLocations();

    List<Location> getLocationsList();

    Location getLocation(int id);

    Location getLocationByFqdn(String fqdn);

    Location getLocationByAddress(String address);

    Location getPrimaryLocation();

    /**
     * Save/update location and publish corresponding save/update events
     *
     * @param location server to save or update
     */
    void saveLocation(Location location);

    void deleteLocation(Location location);
}
