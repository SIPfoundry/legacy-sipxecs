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

import static org.junit.Assert.assertArrayEquals;

import java.util.Collections;

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.ResultDataGrid;

public class DailyBackupScheduleTestIntegration extends IntegrationTestCase {
    private BackupManager m_backupManager;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    public void testStoreJob() throws Exception {
        //ArchiveDefinition d = new ArchiveDefinition("test.tgz", "backup", "restore");
        BackupPlan plan = m_backupManager.findOrCreateBackupPlan(BackupType.local);
        plan.getAutoModeDefinitionIds().add("test.tgz");
        BackupPlan ftpPlan = m_backupManager.findOrCreateBackupPlan(BackupType.ftp);
        ftpPlan.getAutoModeDefinitionIds().add("test.tgz");
        DailyBackupSchedule dailySchedule = new DailyBackupSchedule();
        DailyBackupSchedule ftpDailySchedule = new DailyBackupSchedule();

        plan.addSchedule(dailySchedule);
        ftpPlan.addSchedule(ftpDailySchedule);

        m_backupManager.saveBackupPlan(plan);
        m_backupManager.saveBackupPlan(ftpPlan);

        ResultDataGrid actual = new ResultDataGrid();
        Object[][] expected = new Object[][] {
                {dailySchedule.getId(), false, "Every day"},      
                {ftpDailySchedule.getId(), false, "Every day"}      
        };
        flush();
        db().query("select daily_backup_schedule_id, enabled, scheduled_day from daily_backup_schedule", actual);
        assertArrayEquals(expected, actual.toArray());
    }

    public void setBackupManager(BackupManager backupManager) {
        m_backupManager = backupManager;
    }
}
