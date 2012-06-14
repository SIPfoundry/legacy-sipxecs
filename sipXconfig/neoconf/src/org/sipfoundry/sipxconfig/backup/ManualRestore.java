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
import java.util.Collection;

import org.sipfoundry.sipxconfig.common.WaitingListener;

/**
 * There's no other type other than manual restore, but I wanted name to match ManualBackup
 * because they are similar.
 *
 * Summary of steps to perform a backup: 1.) write custom restore plan in each CFDATA/$location
 * for each location 2.) call sipx-archive to stage files 3.) call restore on all nodes
 */
public class ManualRestore implements WaitingListener {
    private BackupManager m_backupManager;
    private BackupConfig m_backupConfig;

    BackupCommandRunner writePlan(BackupType type, BackupSettings settings) {
        // doesn't matter which plan, we already staged the files
        File planFile = getBackupConfig().writeManualBackupConfigs(type, settings);
        BackupCommandRunner runner = new BackupCommandRunner(planFile, getBackupManager().getBackupScript());
        return runner;
    }

    /**
     * if defIds are null or empty, then files are already staged and we can skip right to node restore
     */
    public void restore(BackupType type, BackupSettings settings, Collection<String> selection) {
        BackupCommandRunner runner = writePlan(type, settings);
        runner.setBackground(true);
        runner.restore(selection);
    }

    /**
     * optionally restore in background, useful when sipxconfig is restoring sipxconfig
     */
    public void restore(BackupType type, BackupSettings settings, Collection<String> selection,
            boolean backgroundProcess) {
        BackupCommandRunner runner = writePlan(type, settings);
        runner.setBackground(backgroundProcess);
        runner.restore(selection);
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

    @Override
    public void afterResponseSent() {
        // not needed as process is kick off in background
    }
}
