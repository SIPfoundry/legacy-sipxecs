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
package org.sipfoundry.sipxconfig.cfgmgt;


import java.io.File;
import java.util.Collection;

import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.alarm.AlarmDefinition;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.feature.FeatureManager;

public interface ConfigManager {
    public static final AddressType SUPERVISOR_ADDRESS = new AddressType("supervisorXmlRpc");
    public static final AlarmDefinition PROCESS_RESTARTED = new AlarmDefinition("PROCESS_RESTARTED");
    public static final AlarmDefinition PROCESS_FAILED = new AlarmDefinition("PROCESS_FAILED");

    public enum ConfigStatus {
        OK, IN_PROGRESS, FAILED
    }

    /**
     * Denote a change in objects related to this feature. Normally code would not have to call
     * this explicitly, but objects should simple implement DeployConfigOnEdit and this call would
     * be made for them.
     *
     * @param feature affected area
     */
    public void configureEverywhere(Feature ... features);

    /**
     * Given a list of locations, filter out the unregistered ones
     */
    public Collection<Location> getRegisteredLocations(Collection<Location> locations);

    /**
     * Of all the locations, only return the registered ones
     */
    public Collection<Location> getRegisteredLocations();

    /**
     * Full resync everywhere. Not to be called frivolously.
     */
    public void configureAllFeaturesEverywhere();

    public void configureAllFeatures(Collection<Location> locations);

    public void run(RunRequest request);

    public File getGlobalDataDirectory();

    public File getLocationDataDirectory(Location location);

    public DomainManager getDomainManager();

    public FeatureManager getFeatureManager();

    public AddressManager getAddressManager();

    public LocationsManager getLocationManager();

    public ConfigCommands getConfigCommands();

    void regenerateMongo(Collection<Location> locations);

    void sendProfiles(Collection<Location> locations);

    void resetKeys(Collection<Location> locations);

    /**
     * Force running fulle configuration cycle now, assuming there is any work to do.
     * Blocks until done and may be a while so you probably want to do this in a thread
     * of you're in web UI.
     */
    public void run();

    /**
     * Only used by setup init task
     */
    public void runProviders();

    public String getRemoteCommand(String server);

}
