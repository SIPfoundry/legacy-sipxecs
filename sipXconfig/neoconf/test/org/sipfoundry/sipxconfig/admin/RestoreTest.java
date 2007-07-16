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

import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.lang.StringUtils;

public class RestoreTest extends TestCase {

    public void testGetCmdLine() {
        Restore restore = new Restore();
        restore.setBinDirectory("");

        BackupBean configBackupBean = new BackupBean();
        configBackupBean.setName("Configuration");
        configBackupBean.setPath("path_to_config_backup");

        List<BackupBean> list = new ArrayList<BackupBean>();
        list.add(configBackupBean);

        String cmdLine = StringUtils.join(restore.getCmdLine(list), ' ');
        assertEquals(restore.getBinDirectory()
                + "/sipx-sudo-restore -c path_to_config_backup --non-interactive", cmdLine);

        BackupBean voicemailBackupBean = new BackupBean();
        voicemailBackupBean.setName("Voicemail");
        voicemailBackupBean.setPath("path_to_mailstore_backup");
        list.add(voicemailBackupBean);

        cmdLine = StringUtils.join(restore.getCmdLine(list), ' ');
        assertEquals(
                restore.getBinDirectory()
                        + "/sipx-sudo-restore -c path_to_config_backup -v path_to_mailstore_backup --non-interactive",
                cmdLine);

    }
}
