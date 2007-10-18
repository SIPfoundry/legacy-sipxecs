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

import org.apache.commons.lang.enums.Enum;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.commserver.Process;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel.ProcessName;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigFileType;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigGenerator;
import org.sipfoundry.sipxconfig.components.EnumPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.site.admin.commserver.RestartReminderPanel;

/**
 * ActivateDialPlan
 */
public abstract class ActivateDialPlan extends PageWithCallback {
    public static final Enum[] OPTIONS = {
        ConfigFileType.MAPPING_RULES, ConfigFileType.FALLBACK_RULES, ConfigFileType.AUTH_RULES,
        ConfigFileType.FORWARDING_RULES
    };

    public static final String PAGE = "ActivateDialPlan";

    public abstract ConfigFileType getSelected();

    public abstract DialPlanContext getDialPlanContext();

    public String getXml() {
        ConfigGenerator generator = getDialPlanContext().getGenerator();
        ConfigFileType type = getSelected();
        return generator.getFileContent(type);
    }

    @SuppressWarnings("unused")
    public void setXml(String xml_) {
        // ignore xml - read only field
    }

    public Process[] getAffectedProcesses() {
        Process[] names = new Process[] {
            new Process(ProcessName.REGISTRAR), new Process(ProcessName.AUTH_PROXY),
            new Process(ProcessName.PROXY)
        };
        return names;
    }

    public IPropertySelectionModel getFileSelectionModel() {
        EnumPropertySelectionModel model = new EnumPropertySelectionModel();
        model.setOptions(OPTIONS);
        return model;
    }

    public void cancel(IRequestCycle cycle) {
        getCallback().performCallback(cycle);
    }

    public void activate(IRequestCycle cycle) {
        DialPlanContext manager = getDialPlanContext();
        manager.activateDialPlan();
        RestartReminderPanel reminder = (RestartReminderPanel) getComponent("reminder");
        reminder.restart();
        getCallback().performCallback(cycle);
    }
}
