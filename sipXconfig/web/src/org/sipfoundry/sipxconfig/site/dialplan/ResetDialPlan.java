/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.components.PageWithCallback;

public abstract class ResetDialPlan extends PageWithCallback {

    public static final String PAGE = "dialplan/ResetDialPlan";

    @InjectObject(value = "spring:dialPlanContext")
    public abstract DialPlanContext getDialPlanContext();

    @InjectObject(value = "spring:localizationContext")
    public abstract LocalizationContext getLocalizationContext();

    public void cancel(IRequestCycle cycle) {
        getCallback().performCallback(cycle);
    }

    public void reset(IRequestCycle cycle) {
        String id = getLocalizationContext().getCurrentRegionId();
        getDialPlanContext().resetToFactoryDefault(id + ".dialPlan");
        getCallback().performCallback(cycle);
        getDialPlanContext().replicateDialPlan(true); // restartSBCDevices == true
    }
}
