/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.List;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.monitoring.MRTGTarget;
import org.sipfoundry.sipxconfig.admin.monitoring.MonitoringContext;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class MonitoringTargetsConfigurationPage extends PageWithCallback implements
        PageBeginRenderListener {
    public static final String PAGE = "admin/MonitoringTargetsConfigurationPage";

    @InjectObject(value = "spring:monitoringContext")
    public abstract MonitoringContext getMonitoringContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract void setTargets(List<MRTGTarget> targets);

    public abstract List<MRTGTarget> getTargets();

    @Persist
    public abstract String getHost();

    public abstract void setHost(String host);

    public abstract void setSelections(SelectMap selections);

    public abstract SelectMap getSelections();

    public void pageBeginRender(PageEvent event) {
        List<MRTGTarget> targets = getMonitoringContext().getTargetsFromTemplate();
        setTargets(targets);
        List<MRTGTarget> selectedTargets = getMonitoringContext().getTargetsForHost(getHost());
        SelectMap selections = new SelectMap();
        for (MRTGTarget selectedTarget : selectedTargets) {
            selections.setSelected(selectedTarget.getTitle(), true);
        }
        setSelections(selections);
    }

    public void editTargets(String host, String returnPage) {
        setHost(host);
        setReturnPage(returnPage);
    }
}
