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
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigGenerator;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.site.admin.commserver.RestartReminderPanel;

/**
 * ActivateDialPlan
 */
public abstract class ActivateDialPlan extends PageWithCallback {
    public static final String[] OPTIONS = {
        "mappingrules.xml.in", "fallbackrules.xml.in", "authrules.xml.in", "forwardingrules.xml.in",
        "nattraversalrules.xml"
    };

    public static final String PAGE = "dialplan/ActivateDialPlan";

    public abstract String getSelected();

    public abstract DialPlanContext getDialPlanContext();

    @InjectObject("spring:sipxRegistrarService")
    public abstract SipxRegistrarService getSipxRegistrarService();

    @InjectObject("spring:sipxProxyService")
    public abstract SipxProxyService getSipxProxyService();

    public String getXml() {
        ConfigGenerator generator = getDialPlanContext().getGenerator();
        String name = getSelected();
        return generator.getFileContent(name);
    }

    @SuppressWarnings("unused")
    public void setXml(String xml_) {
        // ignore xml - read only field
    }

    public SipxService[] getAffectedProcesses() {
        SipxService[] names = new SipxService[] {
            getSipxProxyService(), getSipxRegistrarService()
        };
        return names;
    }

    public IPropertySelectionModel getFileSelectionModel() {
        return new StringPropertySelectionModel(OPTIONS);
    }

    public void cancel(IRequestCycle cycle) {
        getCallback().performCallback(cycle);
    }

    public void activate(IRequestCycle cycle) {
        DialPlanContext manager = getDialPlanContext();
        manager.activateDialPlan(true); // restartSBCDevices == true
        RestartReminderPanel reminder = (RestartReminderPanel) getComponent("reminder");
        reminder.restart();
        getCallback().performCallback(cycle);
    }
}
