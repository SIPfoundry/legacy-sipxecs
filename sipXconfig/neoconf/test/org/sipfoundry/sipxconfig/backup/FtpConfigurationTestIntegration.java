/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.backup;

import java.util.Map;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.ftp.FtpExternalServerConfig;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class FtpConfigurationTestIntegration extends IntegrationTestCase {
    private BackupManager m_backupManager;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    public void testStoreJob() throws Exception {
        FtpBackupPlan ftpPlan = new FtpBackupPlan();
        m_backupManager.storeBackupPlan(ftpPlan);
        
        Map<String, Object> actual = db().queryForMap("select * from backup_plan");
        assertEquals(ftpPlan.getId(), actual.get("backup_plan_id"));
        assertEquals(50, actual.get("limited_count"));
        assertEquals(true, actual.get("configs"));
        assertEquals(true, actual.get("voicemail"));
        
        FtpExternalServerConfig ftpConf = new FtpExternalServerConfig();
        ftpConf.setHost("host");
        ftpConf.setUserId("userId");
        ftpConf.setPassword("password");

        ftpPlan.setFtpConfiguration(ftpConf);
        m_backupManager.storeBackupPlan(ftpPlan);

        Map<String, Object> actualFtp = db().queryForMap("select * from ftp_configuration");
        assertEquals("host" , actualFtp.get("host"));
        assertEquals("userId" , actualFtp.get("user_id"));
        assertEquals("password" , actualFtp.get("password"));
    }
    
    public void setBackupManager(BackupManager backupManager) {
        m_backupManager = backupManager;
    }
}
