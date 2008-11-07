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
import java.io.Writer;
import java.util.Date;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.admin.ftp.FtpConfiguration;

public interface AdminContext {
    public static final String CONTEXT_BEAN_NAME = "adminContext";

    public LocalBackupPlan getLocalBackupPlan();

    public FtpConfiguration getFtpConfiguration();

    public abstract void storeBackupPlan(BackupPlan plan);

    public void storeFtpConfiguration(FtpConfiguration configuration);

    public File[] performBackup(BackupPlan plan);

    public void performExport(Writer writer) throws IOException;

    /**
     * After successfully sending event to application to perform a database related task, remove
     * task from initialization task table.
     */
    public void deleteInitializationTask(String task);

    public String[] getInitializationTasks();

    public List<Map<BackupBean.Type, BackupBean>> getBackups();
    public List<Map<BackupBean.Type, BackupBean>> getFtpBackups();

    public String getBackupDirectory();

    /**
     * @return true if this is an upgrade/data init run, and *not* a real sipXconfig run
     */
    public boolean inUpgradePhase();

    public void setSystemDate(Date date);
}
