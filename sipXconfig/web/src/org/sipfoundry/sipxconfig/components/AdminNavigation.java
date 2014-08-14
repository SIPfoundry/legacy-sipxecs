/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.components;

import java.util.HashSet;
import java.util.Set;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.update.PackageUpdateManager;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class AdminNavigation extends BaseComponent {

    @InjectObject("spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    @InjectObject("spring:coreContext")
    public abstract CoreContext getContext();

    @InjectObject("spring:packageUpdateManager")
    public abstract PackageUpdateManager getPackageUpdateManager();

    @InjectObject("spring:sbcDeviceManager")
    public abstract SbcDeviceManager getSbcDeviceManager();

    @InjectObject("spring:adminContext")
    public abstract AdminContext getAdminContext();

    public abstract Set<String> getEnabled();

    public abstract void setEnabled(Set<String> enabled);

    public boolean isOn(String featureId) {
        return safeGetEnabled().contains(featureId);
    }

    Set<String> safeGetEnabled() {
        Set<String> enabled = getEnabled();
        if (enabled == null) {
            enabled = getEnabledFeatures(getFeatureManager());
        }

        return enabled;
    }

    public static Set<String> getEnabledFeatures(FeatureManager manager) {
        // merge global and local features into one pool
        Set<String> enabled = new HashSet<String>();
        Set<GlobalFeature> global = manager.getEnabledGlobalFeatures();
        for (GlobalFeature f : global) {
            enabled.add(f.getId());
        }
        Set<LocationFeature> local = manager.getEnabledLocationFeatures();
        for (LocationFeature f : local) {
            enabled.add(f.getId());
        }
        return enabled;
    }

    public boolean isSystemAuditEnabled() {
        return getAdminContext().isSystemAuditEnabled();
    }
}
