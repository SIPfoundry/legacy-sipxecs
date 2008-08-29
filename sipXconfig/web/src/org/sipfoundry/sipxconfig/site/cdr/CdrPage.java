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

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcess;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class CdrPage extends BasePage implements PageBeginRenderListener {
    private static final String ACTIVE_TAB = "active";
    private static final String HISTORIC_TAB = "historic";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    @InitialValue(value = "literal:active")
    public abstract String getTab();

    public abstract void setTab(String tab);

    @InjectObject(value = "spring:sipxProcessContext")
    public abstract SipxProcessContext getSipxProcessContext();

    @InjectObject(value = "spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    public abstract SipxProcess getCallResolverProcess();

    public abstract void setCallResolverProcess(SipxProcess sipxProcess);

    public Collection getTabNames() {
        SipxProcess callResolverProcess = getCallResolverProcess();
        if (callResolverProcess.isEnabled()) {
            return Arrays.asList(ACTIVE_TAB, HISTORIC_TAB);
        }
        return Collections.singleton(HISTORIC_TAB);
    }

    public void pageBeginRender(PageEvent event) {
        SipxProcess crp = getCallResolverProcess();
        if (crp == null) {
            crp = new SipxProcess(getLocationsManager(), getSipxProcessContext(),
                    SipxProcessModel.ProcessName.CALL_RESOLVER);
            setCallResolverProcess(crp);
        }

        if (!crp.isEnabled()) {
            setTab(HISTORIC_TAB);
        }
    }
}
