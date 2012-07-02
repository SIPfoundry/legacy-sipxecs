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
package org.sipfoundry.sipxconfig.recording;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.imbot.ImBot;
import org.springframework.beans.factory.annotation.Required;

public class RecordingConfig implements ConfigProvider {
    private Recording m_recording;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(Recording.FEATURE)) {
            return;
        }
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(
                ConferenceBridgeContext.FEATURE);
        if (locations.isEmpty()) {
            return;
        }
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(Recording.FEATURE);
        Address imbotApi = manager.getAddressManager().getSingleAddress(ImBot.REST_API);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            ConfigUtils.enableCfengineClass(dir, "sipxrecording.cfdat", enabled, "sipxrecording");
            if (!enabled) {
                continue;
            }
            File f = new File(dir, "sipxrecording.properties.part");
            Writer wtr = new FileWriter(f);
            try {
                write(wtr, m_recording.getSettings(), imbotApi);
            } finally {
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    void write(Writer wtr, RecordingSettings settings, Address imbotApi) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.equalsSeparated(wtr);
        config.writeSettings(settings.getSettings());
        if (imbotApi != null) {
            config.write("config.sendIMUrl", imbotApi.toString());
        }
    }

    @Required
    public void setRecording(Recording recording) {
        m_recording = recording;
    }
}
