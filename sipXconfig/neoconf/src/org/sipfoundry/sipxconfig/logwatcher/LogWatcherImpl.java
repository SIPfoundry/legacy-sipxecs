/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.logwatcher;

import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;

public class LogWatcherImpl implements LogWatcher, SetupListener {

    @Override
    public void setup(SetupManager manager) {
        if (!manager.isSetup(FEATURE.getId())) {
            manager.getFeatureManager().enableGlobalFeature(FEATURE, true);
            manager.setSetup(FEATURE.getId());
        }
    }

    @Override
    public void nop() {
    }
}
