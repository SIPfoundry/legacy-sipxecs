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
import org.sipfoundry.sipxconfig.admin.ftp.FtpConfiguration;
import org.sipfoundry.sipxconfig.admin.ftp.FtpContext;
import org.springframework.beans.factory.annotation.Required;

public class FtpRestore extends Restore {

    private static final Log LOG = LogFactory.getLog(FtpRestore.class);

    private String m_downloadDirectory;

    private FtpConfiguration m_ftpConfiguration;

    @Override
    protected void prepare(List<BackupBean> backups) {
        // delete previously executed FTP restore
        try {
            FileUtils.deleteDirectory(new File(m_downloadDirectory));
        } catch (IOException ex) {
            LOG.error("Could not delete previously executed FTP restore", ex);
        }
        FtpContext ftpContext = m_ftpConfiguration.getFtpContext();
        try {
            ftpContext.openConnection();
            File backupDownload = new File(m_downloadDirectory);
            backupDownload.mkdir();
            for (BackupBean backupBean : backups) {
                backupBean.getFile().getParentFile().mkdir();
                ftpContext.changeDirectory(backupBean.getParent());
                File localDir = new File(backupDownload, backupBean.getParent());
                ftpContext.download(localDir.getPath(), backupBean.getFile().getName());
                ftpContext.changeDirectory("..");
            }
        } finally {
            ftpContext.closeConnection();
        }
    }

    @Required
    public void setDownloadDirectory(String downloadDirectory) {
        m_downloadDirectory = downloadDirectory;
    }

    public void setFtpConfiguration(FtpConfiguration ftpConfiguration) {
        m_ftpConfiguration = ftpConfiguration;
    }
}
