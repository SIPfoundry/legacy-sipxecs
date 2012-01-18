/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.phonelog;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;

public class PhoneLogConfiguration implements ConfigProvider {

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(PhoneLog.FEATURE)) {
            return;
        }
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(PhoneLog.FEATURE);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            Writer onOff = new FileWriter(new File(dir, "phonelog-config"));
            try {
                write(onOff);
            } finally {
                IOUtils.closeQuietly(onOff);
            }
        }
    }

    void write(Writer wtr) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.colonSeparated(wtr);
        config.write("PHONELOG_ENABLED", "TRUE");
    }
}
