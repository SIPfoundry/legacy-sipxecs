/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.backup;

import java.io.File;

public interface BackupManager {
    public static final String CONTEXT_BEAN_NAME = "backupManager";

    FtpBackupPlan createFtpBackupPlan();

    LocalBackupPlan createLocalBackupPlan();

    BackupPlan getBackupPlan(String type);

    void storeBackupPlan(BackupPlan plan);

    File[] performBackup(BackupPlan plan);

}
