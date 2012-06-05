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
package org.sipfoundry.sipxconfig.site.backup;

import java.io.File;
import java.util.Collection;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.sipfoundry.sipxconfig.backup.BackupCommandRunner;
import org.sipfoundry.sipxconfig.backup.BackupManager;
import org.sipfoundry.sipxconfig.backup.BackupPlan;

public abstract class BackupTable extends BaseComponent {
    @Parameter(required = true)
    public abstract void setBackupPlan(BackupPlan plan);

    @Parameter(defaultValue = "100")
    public abstract void setTableSize(int size);

    public abstract int getTableSize();

    public abstract BackupPlan getBackupPlan();

    @InjectObject("spring:backupManager")
    public abstract BackupManager getBackupManager();

    public abstract String getBackup();

    @Persist()
    public Collection<String> getBackups() {
        File planFile = getBackupManager().getPlanFile(getBackupPlan());
        BackupCommandRunner runner = new BackupCommandRunner(planFile, getBackupManager().getBackupScript());
        List<String> backups = runner.list();
        if (backups.size() > getTableSize()) {
            backups = backups.subList(0, getTableSize());
        }
        return backups;
    }
}
