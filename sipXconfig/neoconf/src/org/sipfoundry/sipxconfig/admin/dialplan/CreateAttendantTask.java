/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import org.sipfoundry.sipxconfig.common.InitTaskListener;
import org.springframework.beans.factory.annotation.Required;

public class CreateAttendantTask extends InitTaskListener {
    private AutoAttendantManager m_autoAttendantManager;

    private DialPlanContext m_dialPlanContext;

    @Override
    public void onInitTask(String task) {
        AutoAttendant attendant = m_autoAttendantManager.createOperator(task);
        if (attendant != null && attendant.isOperator()) {
            m_dialPlanContext.setOperator(attendant);
        }
    }

    @Required
    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    @Required
    public void setAutoAttendantManager(AutoAttendantManager autoAttendantManager) {
        m_autoAttendantManager = autoAttendantManager;
    }
}
