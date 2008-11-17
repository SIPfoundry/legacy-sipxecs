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
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import org.apache.commons.io.FileUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.BackupBean.Type;
import org.sipfoundry.sipxconfig.admin.ftp.FtpConfiguration;
import org.sipfoundry.sipxconfig.admin.ftp.FtpContext;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.test.TestUtil;

import static org.apache.commons.lang.ArrayUtils.contains;

public class FtpBackupPlan extends BackupPlan {
    public static final String TYPE = "F";

    private static final Log LOG = LogFactory.getLog(FtpBackupPlan.class);

    private FtpConfiguration m_ftpConfiguration;

    @Override
    public File[] doPerform(String binPath) throws IOException, InterruptedException {
        File rootBackupDir = TestUtil.createTempDir("ftpBackup");

        File backupDir = createBackupDirectory(rootBackupDir);
        File[] backupFiles = executeBackup(backupDir, new File(binPath));
        uploadBackupsFtp(backupDir);

        FileUtils.deleteDirectory(rootBackupDir);
        return backupFiles;
    }

    @Override
    protected void doPurge(int limitCount) {
        FtpContext ftpContext = m_ftpConfiguration.getFtpContext();
        try {
            ftpContext.openConnection();
            String[] directoriesValid = getValidDirectories(ftpContext);

            int removeCount = directoriesValid.length - limitCount;
            if (removeCount <= 0) {
                return;
            }

            // HACK: sort by name - depends on the fact that name is a nicely formatted dates
            Arrays.sort(directoriesValid);
            for (int i = 0; i < removeCount; i++) {
                try {
                    LOG.info(String.format("Deleting old backup '%s'", directoriesValid[i]));
                    ftpContext.deleteDirectory(directoriesValid[i]);
                } catch (UserException nonfatal) {
                    LOG.error("Could not limit FTP backup count", nonfatal);
                }
            }
        } finally {
            ftpContext.closeConnection();
        }
    }

    private void uploadBackupsFtp(File backupDir) {
        FtpContext ftpContext = m_ftpConfiguration.getFtpContext();
        try {
            ftpContext.openConnection();
            ftpContext.upload(backupDir.getAbsolutePath());
        } finally {
            ftpContext.closeConnection();
        }
    }

    /**
     * Filter mechanism to avoid deleting other directories that are not created by sipXconfig
     * backup.
     */
    public String[] getValidDirectories(FtpContext ftpContext) {
        String[] directories = ftpContext.listDirectories(".");
        List<String> listDirValid = new ArrayList<String>();
        for (String directory : directories) {
            String[] childrenDirectories = ftpContext.listDirectories(directory);
            if (childrenDirectories.length > 0) {
                // skip
                continue;
            }
            String[] childrenFiles = ftpContext.listFiles(directory);
            if (childrenFiles.length > 2) {
                // more than 2 files - not interested
                continue;
            }
            if (contains(childrenFiles, CONFIGURATION_ARCHIVE) || contains(childrenFiles, VOICEMAIL_ARCHIVE)) {
                listDirValid.add(directory);
            }
        }

        String[] directoriesValid = listDirValid.toArray(new String[0]);
        return directoriesValid;
    }

    @Override
    public List<Map<Type, BackupBean>> getBackups() {
        FtpContext ftpContext = m_ftpConfiguration.getFtpContext();
        try {
            ftpContext.openConnection();
            String[] directoryNames = getValidDirectories(ftpContext);

            if (directoryNames == null || directoryNames.length == 0) {
                return Collections.emptyList();
            }
            File[] backupFolders = new File[directoryNames.length];
            int i = 0;
            for (String directoryName : directoryNames) {
                backupFolders[i++] = new File(getBackupDirectory(), directoryName);
            }
            List<Map<Type, BackupBean>> backupBeans = new ArrayList<Map<Type, BackupBean>>();
            for (File backupFolder : backupFolders) {
                String[] names = ftpContext.listFiles(backupFolder.getName());
                List<File> backupFiles = new ArrayList<File>();
                for (String name : names) {
                    if (name.equalsIgnoreCase(BackupPlan.VOICEMAIL_ARCHIVE)
                            || name.equalsIgnoreCase(BackupPlan.CONFIGURATION_ARCHIVE)) {
                        backupFiles.add(new File(backupFolder.getAbsolutePath(), name));
                    }
                }
                addBackupBeans(backupBeans, backupFiles.toArray(new File[backupFiles.size()]));
            }
            Collections.sort(backupBeans, new BackupBean.CompareFolders());
            return backupBeans;
        } finally {
            ftpContext.closeConnection();
        }
    }

    public FtpConfiguration getFtpConfiguration() {
        return m_ftpConfiguration;
    }

    public void setFtpConfiguration(FtpConfiguration ftpConfiguration) {
        m_ftpConfiguration = ftpConfiguration;
    }
}
