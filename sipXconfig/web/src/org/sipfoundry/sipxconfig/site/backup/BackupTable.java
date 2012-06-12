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
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.backup.BackupCommandRunner;
import org.sipfoundry.sipxconfig.backup.BackupManager;
import org.sipfoundry.sipxconfig.backup.BackupPlan;
import org.sipfoundry.sipxconfig.components.SelectMap;

public abstract class BackupTable extends BaseComponent implements PageBeginRenderListener {
    private static final String SPACE = " ";

    @Parameter(required = true)
    public abstract void setBackupPlan(BackupPlan plan);

    @Parameter(required = false, defaultValue = "100")
    public abstract void setTableSize(int size);

    public abstract int getTableSize();

    public abstract BackupPlan getBackupPlan();

    @InjectObject("spring:backupManager")
    public abstract BackupManager getBackupManager();

    public abstract void setBackup(String backup);

    public abstract String getBackup();

    public abstract List<String> getBackups();

    public abstract void setBackups(List<String> backups);

    @Parameter(required = false)
    public abstract void setSelections(SelectMap selections);

    public abstract SelectMap getSelections();

    public abstract void setBackupFile(String backup);

    public abstract String getBackupFile();

    public abstract String getDownloadLinkBase();

    public abstract void setDownloadLinkBase(String base);

    public String getBackupId() {
        return StringUtils.split(getBackup(), SPACE)[0];
    }

    public Collection<String> getBackupFiles() {
        String[] row = StringUtils.split(getBackup(), SPACE);
        if (row.length <= 1) {
            return Collections.emptyList();
        }

        return Arrays.asList(Arrays.copyOfRange(row, 1, row.length));
    }

    public String getDownloadLink() {
        return getDownloadLinkBase() + '/' + getBackupPath();
    }

    public String getBackupPath() {
        return getBackupId() + '/' + getBackupFile();
    }

    public void pageBeginRender(PageEvent event) {
        List<String> backups = getBackups();
        if (backups == null) {
            File planFile = getBackupManager().getPlanFile(getBackupPlan());
            BackupCommandRunner runner = new BackupCommandRunner(planFile, getBackupManager().getBackupScript());
            backups = runner.list();
            if (backups.size() > getTableSize()) {
                backups = backups.subList(0, getTableSize());
            }
            setBackups(backups);
        }

        if (getDownloadLinkBase() == null && !backups.isEmpty()) {
            setDownloadLinkBase(getBackupManager().getBackupLink(getBackupPlan()));
        }
    }
}
