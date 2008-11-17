/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.File;
import java.io.IOException;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.admin.BackupBean.Type;
import org.sipfoundry.sipxconfig.test.TestUtil;

import junit.framework.TestCase;

public class LocalBackupPlanTest extends TestCase {
    private static final String FIRST_BACKUP = "200706101100";

    private static final String SECOND_BACKUP = "200706101101";

    private static final String THIRD_BACKUP = "200706101102";


    @Override
    protected void setUp() throws Exception {
        super.setUp();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testGetBackups() throws Exception {
        // first backup folder;
        buildConfigurationBackup(FIRST_BACKUP);
        buildVoicemailBackup(FIRST_BACKUP);

        // second backup folder;
        buildConfigurationBackup(SECOND_BACKUP);

        // third backup folder;
        buildVoicemailBackup(THIRD_BACKUP);

        BackupPlan backupPlan = new LocalBackupPlan();
        // backup directory is not set up automatically during tests
        backupPlan.setBackupDirectory(getBackupDir());

        List<Map<Type, BackupBean>> backups = backupPlan.getBackups();

        assertEquals(3, backups.size());
        Map<Type, BackupBean> first = backups.get(0);
        assertEquals(2, first.size());
        assertTrue(first.containsKey(Type.CONFIGURATION));
        assertTrue(first.containsKey(Type.VOICEMAIL));
        Map<Type, BackupBean> second = backups.get(1);
        assertEquals(1, second.size());
        assertTrue(second.containsKey(Type.CONFIGURATION));
        assertFalse(second.containsKey(Type.VOICEMAIL));
        Map<Type, BackupBean> third = backups.get(2);
        assertEquals(1, third.size());
        assertFalse(third.containsKey(Type.CONFIGURATION));
        assertTrue(third.containsKey(Type.VOICEMAIL));
    }

    private String getBackupDir() {
        return TestUtil.getTestOutputDirectory("neoconf") + "/backup";
    }

    private void buildConfigurationBackup(String folder) {
        try {
            File file = new File(getBackupDir(), folder);
            file.mkdirs();
            File backup = new File(file, BackupPlan.CONFIGURATION_ARCHIVE);
            backup.createNewFile();
        } catch (IOException ex) {
            fail("Could not create the configs backup");
        }
    }

    private void buildVoicemailBackup(String folder) {
        try {
            File file = new File(getBackupDir(), folder);
            file.mkdirs();
            File backup = new File(file, BackupPlan.VOICEMAIL_ARCHIVE);
            backup.createNewFile();
        } catch (IOException ex) {
            fail("Could not create the mailstore backup");
        }
    }
}
