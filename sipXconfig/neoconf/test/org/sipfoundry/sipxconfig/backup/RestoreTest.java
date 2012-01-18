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

import org.apache.commons.lang.StringUtils;

public class RestoreTest extends TestCase {

    public void testGetCmdLine() {
        File conf = new File("path_to_config_backup", BackupPlan.CONFIGURATION_ARCHIVE);
        BackupBean configBackupBean = new BackupBean(conf);

        String cmdLine = StringUtils.join(Restore.getCmdLine("", configBackupBean, false, false), ' ');
        assertEquals("/sipx-sudo-restore -c " + conf.getAbsolutePath() + " --non-interactive --enforce-version",
                cmdLine);

        cmdLine = StringUtils.join(Restore.getCmdLine("", configBackupBean, true, true), ' ');
        assertEquals("/sipx-sudo-restore -c " + conf.getAbsolutePath()
                + " --non-interactive --enforce-version --verify --no-restart", cmdLine);

        File voicemail = new File("path_to_mailstore_backup", BackupPlan.VOICEMAIL_ARCHIVE);
        BackupBean voicemailBackupBean = new BackupBean(voicemail);

        cmdLine = StringUtils.join(Restore.getCmdLine("", voicemailBackupBean, false, false), ' ');
        assertEquals(
                "/sipx-sudo-restore -v " + voicemail.getAbsolutePath() + " --non-interactive --enforce-version",
                cmdLine);

        cmdLine = StringUtils.join(Restore.getCmdLine("", voicemailBackupBean, true, true), ' ');
        assertEquals("/sipx-sudo-restore -v " + voicemail.getAbsolutePath()
                + " --non-interactive --enforce-version --verify --no-restart", cmdLine);

        File cdr = new File("path_to_cdr_backup", BackupPlan.CDR_ARCHIVE);
        BackupBean cdrBackupBean = new BackupBean(cdr);

        cmdLine = StringUtils.join(Restore.getCmdLine("", cdrBackupBean, false, false), ' ');
        assertEquals("/sipx-sudo-restore -cdr " + cdr.getAbsolutePath() + " --non-interactive --enforce-version",
                cmdLine);

        cmdLine = StringUtils.join(Restore.getCmdLine("", cdrBackupBean, true, true), ' ');
        assertEquals("/sipx-sudo-restore -cdr " + cdr.getAbsolutePath()
                + " --non-interactive --enforce-version --verify --no-restart", cmdLine);

    }
}
