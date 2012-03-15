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
package org.sipfoundry.sipxconfig.site;

import java.util.Set;

import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.AdminNavigation;
import org.sipfoundry.sipxconfig.feature.FeatureManager;

public abstract class InternalPlugins extends BasePage {

    @InjectObject("spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    public abstract Set<String> getEnabled();

    public abstract void setEnabled(Set<String> enabled);

    Set<String> safeGetEnabled() {
        Set<String> enabled = getEnabled();
        if (enabled == null) {
            enabled = AdminNavigation.getEnabledFeatures(getFeatureManager());
        }

        return enabled;
    }

    public boolean isOn(String featureId) {
        return safeGetEnabled().contains(featureId);
    }
}
