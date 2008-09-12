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
import java.util.Arrays;

import org.apache.commons.io.FileUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.ftp.FtpContext;
import org.sipfoundry.sipxconfig.admin.ftp.FtpContextImpl;

public class FtpBackupPlan extends BackupPlan {
    public static final String TYPE = "F";

    private static final Log LOG = LogFactory.getLog(FtpBackupPlan.class);
    private FtpContext m_ftpContext;

    @Override
    public File[] perform(String rootBackupPath, String binPath) {
        String errorMsg = "Errors when creating ftp backup.";
        try {
            File rootBackupDir = new File(rootBackupPath);
            // delete previous FTP backup files
            FileUtils.deleteDirectory(rootBackupDir);

            File backupDir = createBackupDirectory(rootBackupDir);
            File[] backupFiles = executeBackup(backupDir, new File(binPath));
            uploadBackupsFtp(backupDir);
            purgeOld(rootBackupDir);
            return backupFiles;
        } catch (IOException e) {
            LOG.error(errorMsg, e);
        } catch (InterruptedException e) {
            LOG.error(errorMsg, e);
        }
        return null;
    }

    private void uploadBackupsFtp(File backupDir) {
        m_ftpContext.openConnection();
        m_ftpContext.upload(backupDir.getAbsolutePath());
        m_ftpContext.closeConnection();
    }

    @Override
    protected void purgeOld(File rootBackupDir) {
        m_ftpContext.openConnection();
        if (getLimitedCount() == null) {
            return;
        }
        if (getLimitedCount() < 1) {
            // have to leave at least on
            setLimitedCount(1);
        }
        String[] files = m_ftpContext.list(".");
        int removeCount = files.length - getLimitedCount();
        if (removeCount <= 0) {
            return;
        }

        // HACK: sort by name - depends on the fact that name is a nicely formatted dates
        Arrays.sort(files);
        for (int i = 0; i < removeCount; i++) {
            try {
                LOG.info(String.format("Deleting old backup '%s'", files[i]));
                m_ftpContext.delete(files[i]);
            } catch (RuntimeException nonfatal) {
                LOG.error("Could not limit FTP backup count", nonfatal);
            }
        }
        m_ftpContext.closeConnection();
    }

    public FtpContext getFtpContext() {
        if (m_ftpContext == null) {
            m_ftpContext = new FtpContextImpl();
        }
        return m_ftpContext;
    }

    public void setFtpContext(FtpContext ftpContext) {
        m_ftpContext = ftpContext;
    }

}
