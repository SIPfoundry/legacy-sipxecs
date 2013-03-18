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
package org.sipfoundry.sipxconfig.setup;

import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.feature.FeatureManager;

/**
 * Will call all beans that implement SetupListener on startup.  At which point
 * beans can initialize data, set defaults and such
 */
public interface SetupManager {
    public enum Context {
        SETUP_MAIN,
        APP_MAIN
    }

    public ConfigManager getConfigManager();

    public FeatureManager getFeatureManager();

    public boolean isTrue(String id);

    /**
     * This is just !isFlagOn but make code more readable when flags are used like todo list for
     * migration related things instead of done list
     */
    public boolean isFalse(String id);

    public void setTrue(String id);

    /**
     * When you want to mark something as not setup
     */
    public void setFalse(String id);

    public void setup(Context c);

    public Context getContext();
}
