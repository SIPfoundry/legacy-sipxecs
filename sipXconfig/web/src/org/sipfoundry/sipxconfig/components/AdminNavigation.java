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


import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.sipfoundry.sipxconfig.acd.Acd;
import org.sipfoundry.sipxconfig.bridge.BridgeSbcContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.update.PackageUpdateManager;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class AdminNavigation extends BaseComponent {

    @InjectObject("spring:coreContext")
    public abstract CoreContext getContext();

    @InjectObject("spring:packageUpdateManager")
    public abstract PackageUpdateManager getPackageUpdateManager();

    @InjectObject("spring:sbcDeviceManager")
    public abstract SbcDeviceManager getSbcDeviceManager();

    @InjectObject("spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    public boolean isOpenFireEnabled() {
        return getFeatureManager().isFeatureEnabled(ImManager.FEATURE);
    }

    public boolean isAcdEnabled() {
        return getFeatureManager().isFeatureEnabled(Acd.FEATURE);
    }

    public boolean isOpenAcdEnabled() {
        return getFeatureManager().isFeatureEnabled(OpenAcdContext.FEATURE);
    }

    public boolean isSipTrunkEnabled() {
        return getFeatureManager().isFeatureEnabled(BridgeSbcContext.FEATURE);
    }
}
