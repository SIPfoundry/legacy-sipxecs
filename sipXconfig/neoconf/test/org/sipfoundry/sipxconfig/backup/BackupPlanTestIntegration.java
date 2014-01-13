/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.backup;


import java.util.Arrays;
import java.util.Collection;
import java.util.Map;

import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class BackupPlanTestIntegration extends IntegrationTestCase {
    private BackupManager m_backupManager;

    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    public void testStoreJob() throws Exception {
        BackupPlan plan = new BackupPlan(BackupType.local);
        Collection<String> defs = Arrays.asList(AdminContext.ARCHIVE, Ivr.ARCHIVE);
        plan.getDefinitionIds().addAll(defs);
        m_backupManager.saveBackupPlan(plan);
        commit();
        
        Map<String, Object> actual = db().queryForMap("select * from backup_plan");
        assertEquals(plan.getId(), actual.get("backup_plan_id"));
        assertEquals(50, actual.get("limited_count"));        
        assertEquals("configuration.tar.gz,voicemail.tar.gz", actual.get("def"));
        assertEquals("local", actual.get("backup_type"));
    }

    public void setBackupManager(BackupManager backupManager) {
        m_backupManager = backupManager;
    }
}
