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

import java.util.List;

import org.sipfoundry.sipxconfig.nattraversal.NatLocation;
import org.sipfoundry.sipxconfig.service.SipxService;

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
     * Update Nat settings for the corresponding location and publish nat event
     * (we need to separate location save from nat location save because we need
     * different location services set to restart, tracked by nat event)
     * @param location
     * @param nat
     */
    void storeNatLocation(Location location, NatLocation nat);

    /**
     * Saves new location in DB without publishing any events (used on location migration task)
     * (no publish)
     *
     * In case of locations migrated from topology file, we do not want to publish any events
     * related to saving locations, since new locations will be initialized by FirstRunTaks
     *
     * @param location new location migrated from topology file
     */
    void storeMigratedLocation(Location location);

    void deleteLocation(Location location);

    List<Location> getLocationsForService(SipxService service);

    /**
     * Convenience method used only in tests for resetting primary location when needed
     * @see TestPage.resetPrimaryLocation
     */
    public void deletePrimaryLocation();

    /**
     * Returning first location of the server which this bundle is installed on.
     *
     * @param bundleName - service bundle name
     * @return single location of the server which this bundle is installed on. If the bundle
     *         isn't installed on any server it will return null.
     */
    Location getLocationByBundle(String bundleName);
}
