/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setup;

import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.feature.FeatureManager;

/**
 * Will call all beans that implement SetupListener on startup.  At which point
 * beans can initialize data, set defaults and such
 */
public interface SetupManager {

    public ConfigManager getConfigManager();

    public FeatureManager getFeatureManager();

    public boolean isSetup(String id);

    public void setSetup(String id);
}
