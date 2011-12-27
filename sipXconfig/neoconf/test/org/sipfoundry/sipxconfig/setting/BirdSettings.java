/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import org.sipfoundry.sipxconfig.test.TestHelper;

public class BirdSettings extends PersistableSettings {

    @Override
    protected Setting loadSettings() {
        return TestHelper.loadSettings(TestHelper.getResourceAsFile(getClass(), "birds.xml"));
    }

    @Override
    public String getBeanId() {
        return "birdSettings";
    }
}
