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
package org.sipfoundry.sipxconfig.supervisor;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public class SupervisorConfig implements ConfigProvider {
    public static final LocationFeature FEATURE = new LocationFeature("supervisor");

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(FEATURE, LocationsManager.FEATURE)) {
            return;
        }

        File dir = manager.getGlobalDataDirectory();
        Writer w = new FileWriter(new File(dir, "sipxsupervisor-allowed-addrs.part"));
        try {
            for (Location allowed : manager.getLocationManager().getLocations()) {
                w.write(allowed.getAddress());
                w.write("\n");
            }
        } finally {
            IOUtils.closeQuietly(w);
        }
    }
}
