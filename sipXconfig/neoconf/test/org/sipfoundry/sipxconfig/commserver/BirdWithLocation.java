/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.commserver;

import org.sipfoundry.sipxconfig.setting.BirdSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class BirdWithLocation extends SettingsWithLocation {

    @Override
    protected Setting loadSettings() {
        return TestHelper.loadSettings(TestHelper.getResourceAsFile(BirdSettings.class, "birds.xml"));
    }

    @Override
    public String getBeanId() {
        return "birdWithLocation";
    }
}
