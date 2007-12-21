/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.cdr;

import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcess;

public abstract class CdrPage extends BasePage implements PageBeginRenderListener {
    private static final String HISTORIC_TAB = "historic";
    @Persist
    @InitialValue(value = "literal:active")
    public abstract String getTab();

    public abstract void setTab(String tab);

    @InjectObject(value = "spring:callResolverProcess")
    public abstract SipxProcess getCallResolverProcess();

    public void pageBeginRender(PageEvent event) {
        if (!getCallResolverProcess().isEnabled()) {
            setTab(HISTORIC_TAB);
        }
    }
}
