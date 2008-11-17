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

import java.io.File;
import java.io.FileFilter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.filefilter.DirectoryFileFilter;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.BackupBean.Type;

public class LocalBackupPlan extends BackupPlan {
    public static final String TYPE = "L";

    private static final Log LOG = LogFactory.getLog(LocalBackupPlan.class);

    /**
     * Prepares a list of backup files that can be used to restore sipX configuration or sipX
     * voice mail.
     *
     * Each list items is a map: backup type -> backup bean(file name). Each list items contains
     * files from a single directory. List is sorted by directory name in the backup order.
     *
     */
    @Override
    public List<Map<Type, BackupBean>> getBackups() {
        File backupDirectory = new File(getBackupDirectory());
        FileFilter dirFilter = DirectoryFileFilter.DIRECTORY;
        File[] backupFolders = backupDirectory.listFiles(dirFilter);

        if (backupFolders == null) {
            return Collections.emptyList();
        }
        List<Map<Type, BackupBean>> backupBeans = new ArrayList<Map<Type, BackupBean>>();
        for (File backupFolder : backupFolders) {
            File[] backupFiles = backupFolder.listFiles(BackupPlan.BACKUP_FILE_FILTER);
            addBackupBeans(backupBeans, backupFiles);
        }
        Collections.sort(backupBeans, new BackupBean.CompareFolders());
        return backupBeans;
    }

    @Override
    protected void doPurge(int limitCount) {
        File rootBackupDir = new File(getBackupDirectory());
        String[] files = rootBackupDir.list();
        if (files == null) {
            return;
        }
        int removeCount = files.length - limitCount;
        if (removeCount <= 0) {
            return;
        }

        // HACK: sort by name - depends on the fact that name is a nicely formatted dates
        Arrays.sort(files);
        for (int i = 0; i < removeCount; i++) {
            try {
                File oldBackup = new File(rootBackupDir, files[i]);
                LOG.info(String.format("Deleting old backup '%s'", oldBackup));
                FileUtils.deleteDirectory(oldBackup);
            } catch (IOException nonfatal) {
                LOG.error("Could not limit backup count", nonfatal);
            }
        }
    }

    @Override
    public File[] doPerform(String binPath) throws IOException, InterruptedException {
        File rootBackupDir = new File(getBackupDirectory());
        File backupDir = createBackupDirectory(rootBackupDir);
        return executeBackup(backupDir, new File(binPath));
    }
}
