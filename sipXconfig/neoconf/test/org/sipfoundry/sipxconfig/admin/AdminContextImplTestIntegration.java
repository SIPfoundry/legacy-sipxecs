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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import java.io.File;
import java.io.IOException;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.BackupBean.Type;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.Process;
import org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel.ProcessName;
import org.sipfoundry.sipxconfig.admin.ftp.FtpContext;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class AdminContextImplTestIntegration extends IntegrationTestCase {
    private static final String FIRST_BACKUP = "200706101100";

    private static final String SECOND_BACKUP = "200706101101";

    private static final String THIRD_BACKUP = "200706101102";

    private static final String BACKUP_FOLDER = "/backup/";

    private AdminContext m_adminContext;

    public void testGetBackups() throws Exception {
        // first backup folder;
        buildConfigurationBackup(FIRST_BACKUP);
        buildVoicemailBackup(FIRST_BACKUP);

        // second backup folder;
        buildConfigurationBackup(SECOND_BACKUP);

        // third backup folder;
        buildVoicemailBackup(THIRD_BACKUP);

        List<Map<Type, BackupBean>> backups = m_adminContext.getBackups();

        for (Map<Type, BackupBean> map : backups) {
            System.err.println(map.entrySet().iterator().next().getValue().getParent());
        }

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

    public void testGetFtpBackups() {
        FtpContext ftpContext = createMock(FtpContext.class);

        String [] directoryNames = new String [] {"200804221100", "200804221101", "200804221102"};
        String [] names1 = new String [] {BackupPlan.CONFIGURATION_ARCHIVE, BackupPlan.VOICEMAIL_ARCHIVE};
        String [] names2 = new String [] {BackupPlan.CONFIGURATION_ARCHIVE};
        String [] names3 = new String [] {BackupPlan.VOICEMAIL_ARCHIVE};

        ftpContext.setHost(null);
        ftpContext.setUserId(null);
        ftpContext.setPassword(null);

        ftpContext.openConnection();

        expect(ftpContext.list(".")).andReturn(directoryNames).once();
        expect(ftpContext.list(directoryNames[0])).andReturn(names1).once();
        expect(ftpContext.list(directoryNames[1])).andReturn(names2).once();
        expect(ftpContext.list(directoryNames[2])).andReturn(names3).once();

        ftpContext.closeConnection();

        replay(ftpContext);

        ((FtpBackupPlan) m_adminContext.getFtpConfiguration().getBackupPlan()).setFtpContext(ftpContext);

        List<Map<Type, BackupBean>> backups = m_adminContext.getFtpBackups();

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

        verify(ftpContext);
    }
    public void setAdminContext(AdminContext adminContext) {
        m_adminContext = adminContext;
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
