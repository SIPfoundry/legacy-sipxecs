package org.sipfoundry.sipxconfig.backup;

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class BackupTestIntegration extends IntegrationTestCase {
    private BackupSettings m_backupSettings;
    private BackupManager m_backupManager;
    
    public void testSettings() {
        System.out.println(m_backupSettings);
        BackupSettings x = m_backupManager.getSettings();
        System.out.println(x);
        m_backupManager.saveSettings(x);
        BackupSettings y = m_backupManager.getSettings();
        System.out.println(y);        
    }

    public void setBackupSettings(BackupSettings backupSettings) {
        m_backupSettings = backupSettings;
    }

    public void setBackupManager(BackupManager backupManager) {
        m_backupManager = backupManager;
    }       
}
