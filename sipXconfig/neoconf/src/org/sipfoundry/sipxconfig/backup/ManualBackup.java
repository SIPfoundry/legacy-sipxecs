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

import static java.lang.String.format;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Collection;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;

public class ManualBackup {
    private BackupManager m_backupManager;
    private BackupConfig m_backupConfig;
    private LocationsManager m_locationsManager;
    private ConfigManager m_configManager;

    public void backup(BackupPlan plan) {
        BackupSettings settings = m_backupManager.getSettings();
        backup(plan, settings);
    }

    /**
     * Summary of steps to perform a backup:
     * 1.) write custom backup plan in each CFDATA/$location for each location
     * 2.) write custom cluster backup plan in CFDATA/primary location
     * 3.) call cluster backup script for plan
     * 4.) check for new backup, otherwise there was a failure
     */
    public void backup(BackupPlan plan, BackupSettings settings) {
        String planId = "manual";

        List<Location> locations = m_locationsManager.getLocationsList();
        boolean atLeastOneBackupToDo = false;
        for (Location location : locations) {
            Collection<ArchiveDefinition> defs = m_backupManager.getArchiveDefinitions(plan.getDefinitionIds(),
                    location);
            File dir = m_configManager.getLocationDataDirectory(location);
            Writer w = null;
            try {
                w = new FileWriter(new File(dir, format("backup-%s.yaml", planId)));
                m_backupConfig.writeBackupConfig(w, defs);
            } catch (IOException e) {
                throw new UserException("Failed to create backup plan", e);
            } finally {
                IOUtils.closeQuietly(w);
            }
            atLeastOneBackupToDo = atLeastOneBackupToDo || !defs.isEmpty();
        }

        if (!atLeastOneBackupToDo) {
            throw new UserException("No backups to perform");
        }

        Location primary = m_locationsManager.getPrimaryLocation();
        File dir = m_configManager.getLocationDataDirectory(primary);
        File planFile = new File(dir, "backup-cluster-" + planId + ".yaml");
        Writer w = null;
        try {
            w = new FileWriter(planFile);
            m_backupConfig.writeClusterBackupConfig(w, plan, locations, settings);
        } catch (IOException e) {
            throw new UserException("Failed to create cluster backup plan", e);
        } finally {
            IOUtils.closeQuietly(w);
        }

        BackupCommandRunner runner = new BackupCommandRunner(planFile, m_backupManager.getBackupScript());
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

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }
}
