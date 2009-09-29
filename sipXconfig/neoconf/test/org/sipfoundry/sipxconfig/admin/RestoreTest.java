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
import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.lang.StringUtils;

public class RestoreTest extends TestCase {

    public void testGetCmdLine() {
        Restore restore = new Restore();
        restore.setBinDirectory("");

        File conf = new File("path_to_config_backup", BackupPlan.CONFIGURATION_ARCHIVE);
        BackupBean configBackupBean = new BackupBean(conf);

        List<BackupBean> list = new ArrayList<BackupBean>();
        list.add(configBackupBean);

        String cmdLine = StringUtils.join(restore.getCmdLine(list, false), ' ');
        assertEquals(restore.getBinDirectory() + "/sipx-sudo-restore -c "
                + conf.getAbsolutePath() + " --non-interactive --enforce-version", cmdLine);

        cmdLine = StringUtils.join(restore.getCmdLine(list, true), ' ');
        assertEquals(restore.getBinDirectory() + "/sipx-sudo-restore -c "
                + conf.getAbsolutePath() + " --non-interactive --enforce-version --verify",
                cmdLine);

        File voicemail = new File("path_to_mailstore_backup", BackupPlan.VOICEMAIL_ARCHIVE);
        BackupBean voicemailBackupBean = new BackupBean(voicemail);
        list.add(voicemailBackupBean);

        cmdLine = StringUtils.join(restore.getCmdLine(list, false), ' ');
        assertEquals(restore.getBinDirectory() + "/sipx-sudo-restore -c "
                + conf.getAbsolutePath() + " -v " + voicemail.getAbsolutePath()
                + " --non-interactive --enforce-version", cmdLine);

        cmdLine = StringUtils.join(restore.getCmdLine(list, true), ' ');
        assertEquals(restore.getBinDirectory() + "/sipx-sudo-restore -c "
                + conf.getAbsolutePath() + " -v " + voicemail.getAbsolutePath()
                + " --non-interactive --enforce-version --verify", cmdLine);

    }
}
