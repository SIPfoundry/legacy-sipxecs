/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.backup;

import java.io.File;

import junit.framework.TestCase;

public class RestoreTest extends TestCase {

    public void testGetCmdLine() {
        Restore restore = new Restore();
        File conf = new File("path_to_config_backup", BackupPlan.CONFIGURATION_ARCHIVE);
        BackupBean configBackupBean = new BackupBean(conf);

        String cmdLine = restore.composeCmdLine(configBackupBean, false, false);
        assertEquals("-c " + conf.getAbsolutePath() + " --non-interactive --enforce-version", cmdLine);

        cmdLine = restore.composeCmdLine(configBackupBean, true, true);
        assertEquals("-c " + conf.getAbsolutePath() + " --non-interactive --enforce-version --verify --no-restart",
                cmdLine);

        File voicemail = new File("path_to_mailstore_backup", BackupPlan.VOICEMAIL_ARCHIVE);
        BackupBean voicemailBackupBean = new BackupBean(voicemail);

        cmdLine = restore.composeCmdLine(voicemailBackupBean, false, false);
        assertEquals("-v " + voicemail.getAbsolutePath() + " --non-interactive --enforce-version", cmdLine);

        cmdLine = restore.composeCmdLine(voicemailBackupBean, true, true);
        assertEquals("-v " + voicemail.getAbsolutePath()
                + " --non-interactive --enforce-version --verify --no-restart", cmdLine);

        File cdr = new File("path_to_cdr_backup", BackupPlan.CDR_ARCHIVE);
        BackupBean cdrBackupBean = new BackupBean(cdr);

        cmdLine = restore.composeCmdLine(cdrBackupBean, false, false);
        assertEquals("-cdr " + cdr.getAbsolutePath() + " --non-interactive --enforce-version", cmdLine);

        cmdLine = restore.composeCmdLine(cdrBackupBean, true, true);
        assertEquals("-cdr " + cdr.getAbsolutePath() + " --non-interactive --enforce-version --verify --no-restart",
                cmdLine);

    }
}
