/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.ftp.FtpConfiguration;

public class AdminContextImplTestIntegration extends IntegrationTestCase {

    private AdminContext m_adminContext;

    public void setAdminContext(AdminContext adminContext) {
        m_adminContext = adminContext;
    }

    public void testGetLocalBackupPlan() {
        BackupPlan backupPlan = m_adminContext.getBackupPlan(LocalBackupPlan.TYPE);
        assertTrue(backupPlan instanceof LocalBackupPlan);
    }

    public void testGetFtpBackupPlan() {
        BackupPlan backupPlan = m_adminContext.getBackupPlan(FtpBackupPlan.TYPE);
        assertTrue(backupPlan instanceof FtpBackupPlan);

        FtpBackupPlan ftpBackupPlan = (FtpBackupPlan) backupPlan;
        FtpConfiguration ftpConfiguration = ftpBackupPlan.getFtpConfiguration();
        assertNotNull(ftpConfiguration);
        assertFalse(ftpConfiguration.isNew());

        flush();

        ftpConfiguration.setHost("ftp.example.com");
        ftpConfiguration.setPassword("password");
        ftpConfiguration.setUserId("xxx");

        m_adminContext.storeBackupPlan(backupPlan);

        BackupPlan backupPlan1 = m_adminContext.getBackupPlan(FtpBackupPlan.TYPE);
        assertSame(backupPlan1, backupPlan);
    }

    public void testInUpgradePhase() {
        assertFalse(m_adminContext.inInitializationPhase());

        System.setProperty("sipxconfig.initializationPhase", "true");
        assertTrue(m_adminContext.inInitializationPhase());

        System.clearProperty("sipxconfig.initializationPhase");
    }

    public void testStoreLocalBackupPlan() {
        BackupPlan backupPlan = m_adminContext.getBackupPlan(LocalBackupPlan.TYPE);

        backupPlan.setEmailAddress("root@example.com");
        backupPlan.addSchedule(new DailyBackupSchedule());
        assertEquals(1, backupPlan.getSchedules().size());

        m_adminContext.storeBackupPlan(backupPlan);
    }
}
