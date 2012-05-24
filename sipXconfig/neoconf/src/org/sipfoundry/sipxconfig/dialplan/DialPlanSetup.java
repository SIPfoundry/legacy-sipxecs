/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.dialplan;

import org.sipfoundry.sipxconfig.setup.MigrationListener;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.springframework.beans.factory.annotation.Required;

public class DialPlanSetup implements SetupListener, MigrationListener {
    private AutoAttendantManager m_autoAttendantManager;
    private DialPlanContext m_dialPlanContext;
    private String m_defaultDialPlanId = "na.dialPlan";

    @Required
    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    @Required
    public void setAutoAttendantManager(AutoAttendantManager autoAttendantManager) {
        m_autoAttendantManager = autoAttendantManager;
    }

    public void setupDefaultRegion() {
        setup(m_defaultDialPlanId);
    }

    public void setup(String regionId) {
        AutoAttendant operator = m_autoAttendantManager.getOperator();
        m_dialPlanContext.resetToFactoryDefault(regionId, operator);
    }

    @Override
    public void setup(SetupManager manager) {
        setupRegion(manager);
        setupAttendant(manager);
    }

    void setupRegion(SetupManager manager) {
        if (manager.isTrue(m_defaultDialPlanId)) {
            return;
        }
        if (!m_dialPlanContext.isInitialized()) {
            setupDefaultRegion();
        }
        manager.setTrue(m_defaultDialPlanId);
    }

    void setupAttendant(SetupManager manager) {
        for (String id : new String[] {
            "afterhour", "operator"
        }) {
            if (manager.isFalse(id)) {
                AutoAttendant operator = m_autoAttendantManager.createOperator(id);
                if (operator != null && operator.isOperator()) {
                    m_dialPlanContext.setOperator(operator);
                }
                manager.setTrue(id);
            }
        }
    }

    void migrateDialPlanGaps(SetupManager manager) {
        String id = "cleanup-dial-plans";
        if (manager.isTrue(id)) {
            return;
        }

        m_dialPlanContext.removeEmptyRules();
        manager.setTrue(id);
    }

    @Override
    public void migrate(SetupManager manager) {
        migrateDialPlanGaps(manager);
    }

    public void setDefaultDialPlanId(String defaultDialPlanId) {
        m_defaultDialPlanId = defaultDialPlanId;
    }
}
