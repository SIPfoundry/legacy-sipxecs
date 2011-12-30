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

import org.sipfoundry.sipxconfig.ftp.FtpConfiguration;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class BackupManagerTestIntegration extends IntegrationTestCase {
    private BackupManager m_backupManager;
    
    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    public void testGetLocalBackupPlan() {
        BackupPlan backupPlan = m_backupManager.getBackupPlan(LocalBackupPlan.TYPE);
        assertTrue(backupPlan instanceof LocalBackupPlan);
    }

    public void testGetFtpBackupPlan() {
        BackupPlan backupPlan = m_backupManager.getBackupPlan(FtpBackupPlan.TYPE);
        assertTrue(backupPlan instanceof FtpBackupPlan);

        FtpBackupPlan ftpBackupPlan = (FtpBackupPlan) backupPlan;
        FtpConfiguration ftpConfiguration = ftpBackupPlan.getFtpConfiguration();
        assertNotNull(ftpConfiguration);
        assertFalse(ftpConfiguration.isNew());

        flush();

        ftpConfiguration.setHost("ftp.example.com");
        ftpConfiguration.setPassword("password");
        ftpConfiguration.setUserId("xxx");

        m_backupManager.storeBackupPlan(backupPlan);

        BackupPlan backupPlan1 = m_backupManager.getBackupPlan(FtpBackupPlan.TYPE);
        assertSame(backupPlan1, backupPlan);
    }

    public void testStoreLocalBackupPlan() {
        BackupPlan backupPlan = m_backupManager.getBackupPlan(LocalBackupPlan.TYPE);

        backupPlan.setEmailAddress("root@example.com");
        backupPlan.addSchedule(new DailyBackupSchedule());
        assertEquals(1, backupPlan.getSchedules().size());

        m_backupManager.storeBackupPlan(backupPlan);
    }

    public void setBackupManager(BackupManager backupManager) {
        m_backupManager = backupManager;
    }
}
