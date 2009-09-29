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

public interface AdminContext {
    final String CONTEXT_BEAN_NAME = "adminContext";

    BackupPlan getBackupPlan(String type);

    void storeBackupPlan(BackupPlan plan);

    File[] performBackup(BackupPlan plan);

    void performExport(Writer writer) throws IOException;

    /**
     * After successfully sending event to application to perform a database related task, remove
     * task from initialization task table.
     */
    void deleteInitializationTask(String task);

    String[] getInitializationTasks();

    public String getBinDirectory();
    public String getLibExecDirectory();

    /**
     * @return true if this is an upgrade/data init run, and *not* a real sipXconfig run
     */
    boolean inInitializationPhase();

}
