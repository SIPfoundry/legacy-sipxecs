/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.recording;

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
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;

public class RecordingConfiguration implements ConfigProvider {
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

        RecordingSettings settings = m_recording.getSettings();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            Writer writer = new FileWriter(new File(dir, "sipxrecording.properties"));
            try {
                write(writer, settings);
            } finally {
                IOUtils.closeQuietly(writer);
            }
        }
    }

    void write(Writer wtr, RecordingSettings settings) throws IOException {
        KeyValueConfiguration config = new KeyValueConfiguration(wtr, "=");
        config.write(settings.getSettings().getSetting("recording"));
    }

    public void setRecording(Recording recording) {
        m_recording = recording;
    }
}
