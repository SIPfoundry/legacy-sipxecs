/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver;

public interface LocationsManager {
    String CONTEXT_BEAN_NAME = "locationsManager";

    Location[] getLocations();

    Location getLocation(int id);

    Location getLocationByFqdn(String fqdn);

    Location getLocationByAddress(String address);

    Location getPrimaryLocation();

    /**
     * Save/update location and publish corresponding save/update events
     *
     * @param location server to save or update
     */
    void storeLocation(Location location);

    /**
     * Saves new location in DB without publishing any events (used on location migration task)
     * (no publish)
     *
     * In case of locations migrated from topology file, we do not want to publish any events
     * related to saving locations, since new locations will be initialized by FirstRunTaks
     *
     * @param location new location migrated from topology file
     */
    void saveMigratedLocation(Location location);

    void deleteLocation(Location location);
}
