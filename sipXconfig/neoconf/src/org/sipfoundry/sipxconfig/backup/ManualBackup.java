/**
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.backup;

import java.io.File;

import org.sipfoundry.sipxconfig.common.UserException;

/**
 * Summary of steps to perform a backup:
 * 1.) write custom backup plan in each CFDATA/$location for each location
 * 2.) write custom cluster backup plan in CFDATA/primary location
 * 3.) call cluster backup script for plan
 * 4.) check for new backup, otherwise there was a failure
 */
public class ManualBackup {
    private BackupManager m_backupManager;
    private BackupConfig m_backupConfig;

    public void backup(BackupPlan plan) {
        BackupSettings settings = getBackupManager().getSettings();
        backup(plan, settings);
    }

    public void backup(BackupPlan plan, BackupSettings settings) {
        File planFile = getBackupConfig().writeManualBackupConfigs(plan, settings);
        BackupCommandRunner runner = new BackupCommandRunner(planFile, getBackupManager().getBackupScript());
        String beforeBackup = runner.lastBackup(); // null is ok
        runner.backup();
        String afterBackup = runner.lastBackup();
        if (afterBackup.equals(beforeBackup)) {
            throw new UserException("Failed to perform backup");
        }
    }

    public void setBackupManager(BackupManager backupManager) {
        m_backupManager = backupManager;
    }
    public void setBackupConfig(BackupConfig backupConfig) {
        m_backupConfig = backupConfig;
    }

    public BackupManager getBackupManager() {
        return m_backupManager;
    }

    public BackupConfig getBackupConfig() {
        return m_backupConfig;
    }
}
