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

public class ResetDialPlanTask extends InitTaskListener {
    private AutoAttendantManager m_autoAttendantManager;

    private DialPlanContext m_dialPlanContext;

    private String m_defaultDialPlanId;

    @Override
    public void onInitTask(String task) {
        reset(true);
    }

    public void reset(boolean force) {
        if (force || !m_dialPlanContext.isInitialized()) {
            reset(m_defaultDialPlanId);
        }
    }

    public void reset(String dialPlanBeanName) {
        AutoAttendant operator = m_autoAttendantManager.getOperator();
        m_dialPlanContext.resetToFactoryDefault(dialPlanBeanName, operator);
    }

    @Required
    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    @Required
    public void setAutoAttendantManager(AutoAttendantManager autoAttendantManager) {
        m_autoAttendantManager = autoAttendantManager;
    }

    @Required
    public void setDefaultDialPlanId(String defaultDialPlanId) {
        m_defaultDialPlanId = defaultDialPlanId;
    }
}
