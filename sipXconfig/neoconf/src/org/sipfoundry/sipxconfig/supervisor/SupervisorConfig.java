/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
