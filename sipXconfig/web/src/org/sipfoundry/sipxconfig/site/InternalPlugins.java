/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site;

import java.util.HashSet;
import java.util.Set;

import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public abstract class InternalPlugins extends BasePage {

    @InjectObject("spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    public abstract Set<String> getEnabled();

    public abstract void setEnabled(Set<String> enabled);

    Set<String> safeGetEnabled() {
        Set<String> enabled = getEnabled();
        if (enabled == null) {
            enabled = new HashSet<String>();
            Set<GlobalFeature> global = getFeatureManager().getEnabledGlobalFeatures();
            for (GlobalFeature f : global) {
                enabled.add(f.getId());
            }
            Set<LocationFeature> local = getFeatureManager().getEnabledLocationFeatures();
            for (LocationFeature f : local) {
                enabled.add(f.getId());
            }
        }

        return enabled;
    }

    public boolean isOn(String featureId) {
        return safeGetEnabled().contains(featureId);
    }
}
