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
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.components.PageWithCallback;

public abstract class ResetDialPlan extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "ResetDialPlan";

    public abstract DialPlanContext getDialPlanContext();

    public abstract String getTemplate();

    public abstract void setTemplate(String template);

    public void pageBeginRender(PageEvent event) {
        if (getTemplate() == null) {
            String defaultTemplate = getDialPlanContext().getDefaultDialPlanId();
            setTemplate(defaultTemplate);
        }
    }

    public void cancel(IRequestCycle cycle) {
        getCallback().performCallback(cycle);
    }

    public void reset(IRequestCycle cycle) {
        DialPlanContext manager = getDialPlanContext();
        manager.resetToFactoryDefault(getTemplate());
        getCallback().performCallback(cycle);
    }
}
