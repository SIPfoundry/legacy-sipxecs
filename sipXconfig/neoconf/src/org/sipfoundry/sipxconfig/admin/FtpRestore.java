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
import java.util.List;

import org.apache.commons.io.FileUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.ftp.FtpContext;
import org.sipfoundry.sipxconfig.admin.ftp.FtpContextImpl;

public class FtpRestore extends Restore {

    private static final Log LOG = LogFactory.getLog(FtpRestore.class);

    private FtpContext m_ftpContext;
    private String m_downloadDirectory;

    public void afterResponseSent() {
        perform(getSelectedBackups());
    }

    public void perform(List<BackupBean> backups) {
        downloadFtpBackups(backups);
        execute(backups, false);
    }

    public void validate(List<BackupBean> backups) {
        downloadFtpBackups(backups);
        execute(backups, true);
    }

    private void downloadFtpBackups(List<BackupBean> backups) {
        //delete previously executed FTP restore
        try {
            FileUtils.deleteDirectory(new File(getDownloadDirectory()));
        } catch (IOException ex) {
            LOG.error("Could not delete previously executed FTP restore", ex);
        }
        m_ftpContext.openConnection();
        File backupDownload = new File(getDownloadDirectory());
        backupDownload.mkdir();
        for (BackupBean backupBean : backups) {
            backupBean.getFile().getParentFile().mkdir();
            m_ftpContext.changeDirectory(backupBean.getParent());
            m_ftpContext.download(getDownloadDirectory() + File.separator + backupBean.getParent(),
                    backupBean.getFile().getName());
            m_ftpContext.changeDirectory("..");
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

    public String getDownloadDirectory() {
        return m_downloadDirectory;
    }
    public void setDownloadDirectory(String downloadDirectory) {
        m_downloadDirectory = downloadDirectory;
    }

}
