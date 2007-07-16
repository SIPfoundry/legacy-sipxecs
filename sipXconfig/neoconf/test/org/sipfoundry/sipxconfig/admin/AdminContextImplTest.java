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

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.test.TestUtil;
import org.springframework.context.ApplicationContext;

public class AdminContextImplTest extends TestCase {
    private static final String FIRST_BACKUP = "200706101100";

    private static final String SECOND_BACKUP = "200706101101";

    private static final String THIRD_BACKUP = "200706101102";

    private static final String BACKUP_FOLDER = "/backup/";

    private AdminContext m_adminContext;

    protected void setUp() {
        // first backup folder;
        buildConfigurationBackup(FIRST_BACKUP);
        buildVoicemailBackup(FIRST_BACKUP);

        // second backup folder;
        buildConfigurationBackup(SECOND_BACKUP);

        // third backup folder;
        buildVoicemailBackup(THIRD_BACKUP);
    }

    public void testGetBackups() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_adminContext = (AdminContext) appContext.getBean(AdminContext.CONTEXT_BEAN_NAME);
        BackupBean[] backups = m_adminContext.getBackups();
        assertEquals(4, backups.length);
    }

    private void buildConfigurationBackup(String folder) {
        String testFolder = TestUtil.getTestOutputDirectory("neoconf");
        String configBackup = testFolder + BACKUP_FOLDER + folder;
        try {
            File file = new File(configBackup);
            file.mkdirs();
            File backup = new File(file, BackupPlan.CONFIGURATION_ARCHIVE);
            backup.createNewFile();
        } catch (IOException ex) {
            fail("Could not create the configs backup");
        }

    }

    private void buildVoicemailBackup(String folder) {
        String testFolder = TestUtil.getTestOutputDirectory("neoconf");
        String voicemailBackup = testFolder + BACKUP_FOLDER + folder;
        try {
            File file = new File(voicemailBackup);
            file.mkdirs();
            File backup = new File(file, BackupPlan.VOICEMAIL_ARCHIVE);
            backup.createNewFile();
        } catch (IOException ex) {
            fail("Could not create the mailstore backup");
        }
    }
}
