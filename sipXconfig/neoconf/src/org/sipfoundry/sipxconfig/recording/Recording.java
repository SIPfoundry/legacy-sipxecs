/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.recording;

import org.sipfoundry.sipxconfig.feature.GlobalFeature;

public interface Recording {
    public static final GlobalFeature FEATURE = new GlobalFeature("bridgeRecording");

    public RecordingSettings getSettings();

    public void saveSettings(RecordingSettings settings);
}
