/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
