/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.dialplan.DialPlanActivationManager;
import org.sipfoundry.sipxconfig.dialplan.DialPlanSetup;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;

public abstract class ResetDialPlan extends PageWithCallback {

    public static final String PAGE = "dialplan/ResetDialPlan";

    @InjectObject("spring:localizationContext")
    public abstract LocalizationContext getLocalizationContext();

    @InjectObject("spring:dialPlanActivationManager")
    public abstract DialPlanActivationManager getDialPlanActivationManager();

    @InjectObject("spring:resetDialPlanTask")
    public abstract DialPlanSetup getResetDialPlanTask();

    public void cancel(IRequestCycle cycle) {
        getCallback().performCallback(cycle);
    }

    public void reset(IRequestCycle cycle) {
        String id = getLocalizationContext().getCurrentRegionId();
        getResetDialPlanTask().setup(id + ".dialPlan");
        getCallback().performCallback(cycle);
        getDialPlanActivationManager().replicateDialPlan(true);
    }
}
