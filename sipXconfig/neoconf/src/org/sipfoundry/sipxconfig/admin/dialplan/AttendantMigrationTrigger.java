/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import org.sipfoundry.sipxconfig.common.InitTaskListener;

public class AttendantMigrationTrigger extends InitTaskListener {
    private AttendantMigrationContext m_attendantMigrationContext;

    public void setAttendantMigrationContext(AttendantMigrationContext attendantMigrationContext) {
        m_attendantMigrationContext = attendantMigrationContext;
    }

    @Override
    public void onInitTask(String task) {
        if ("dial_plan_migrate_attendant_rules".equals(task)) {
            m_attendantMigrationContext.migrateAttendantRules();
        } else if ("attendant_defaults".equals(task)) {
            m_attendantMigrationContext.setAttendantDefaults();
        }
    }
}
