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
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivationManager;
import org.sipfoundry.sipxconfig.admin.dialplan.ResetDialPlanTask;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.components.PageWithCallback;

public abstract class ResetDialPlan extends PageWithCallback {

    public static final String PAGE = "dialplan/ResetDialPlan";

    @InjectObject("spring:localizationContext")
    public abstract LocalizationContext getLocalizationContext();

    @InjectObject("spring:dialPlanActivationManager")
    public abstract DialPlanActivationManager getDialPlanActivationManager();

    @InjectObject("spring:resetDialPlanTask")
    public abstract ResetDialPlanTask getResetDialPlanTask();

    public void cancel(IRequestCycle cycle) {
        getCallback().performCallback(cycle);
    }

    public void reset(IRequestCycle cycle) {
        String id = getLocalizationContext().getCurrentRegionId();
        getResetDialPlanTask().reset(id + ".dialPlan");
        getCallback().performCallback(cycle);
        getDialPlanActivationManager().replicateDialPlan(true);
    }
}
