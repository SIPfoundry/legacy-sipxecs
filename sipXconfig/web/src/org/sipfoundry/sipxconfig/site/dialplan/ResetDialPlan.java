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
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.components.PageWithCallback;

public abstract class ResetDialPlan extends PageWithCallback implements PageBeginRenderListener {

    public abstract DialPlanContext getDialPlanContext();

    @InjectObject(value = "spring:localizationContext")
    public abstract LocalizationContext getLocalizationContext();

    public abstract String getTemplate();

    public abstract void setTemplate(String template);

    public void pageBeginRender(PageEvent event) {
        if (getTemplate() == null) {
            // Get the name of the template corresponding to the current region
            LocalizationContext localizationContext = getLocalizationContext();
            if (localizationContext != null) {
                String id = localizationContext.getCurrentRegionId();
                if (id != null) {
                    setTemplate(id + ".dialPlan");
                }
            }
        }
    }

    public void cancel(IRequestCycle cycle) {
        getCallback().performCallback(cycle);
    }

    public void reset(IRequestCycle cycle) {
        String defaultTemplate = getTemplate();
        if (defaultTemplate == null) {
            getDialPlanContext().resetToFactoryDefault();
        } else {
            getDialPlanContext().resetToFactoryDefault(defaultTemplate);
        }
        getCallback().performCallback(cycle);
    }
}
