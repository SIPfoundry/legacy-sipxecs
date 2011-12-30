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

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.ResultDataGrid;

public class DailyBackupScheduleTestDb extends IntegrationTestCase {
    private BackupManager m_backupManager;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    public void testStoreJob() throws Exception {
        BackupPlan plan = m_backupManager.getBackupPlan(LocalBackupPlan.TYPE);
        BackupPlan ftpPlan = m_backupManager.getBackupPlan(FtpBackupPlan.TYPE);
        DailyBackupSchedule dailySchedule = new DailyBackupSchedule();
        DailyBackupSchedule ftpDailySchedule = new DailyBackupSchedule();

        plan.addSchedule(dailySchedule);
        ftpPlan.addSchedule(ftpDailySchedule);

        m_backupManager.storeBackupPlan(plan);
        m_backupManager.storeBackupPlan(ftpPlan);

        ResultDataGrid actual = new ResultDataGrid();
        Object[][] expected = new Object[][] {
                {dailySchedule.getId(), false, "Every day"},      
                {ftpDailySchedule.getId(), false, "Every day"}      
        };
        db().query("select daily_backup_schedule_id, enabled, scheduled_day from daily_backup_schedule", actual);
        assertArrayEquals(expected, actual.toArray());
    }

    public void setBackupManager(BackupManager backupManager) {
        m_backupManager = backupManager;
    }
}
