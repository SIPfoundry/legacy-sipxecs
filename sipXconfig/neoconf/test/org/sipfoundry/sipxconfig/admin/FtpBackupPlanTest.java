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

import java.util.List;
import java.util.Map;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.BackupBean.Type;
import org.sipfoundry.sipxconfig.admin.ftp.FtpConfiguration;
import org.sipfoundry.sipxconfig.admin.ftp.FtpContext;

import static org.apache.commons.lang.ArrayUtils.EMPTY_STRING_ARRAY;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class FtpBackupPlanTest extends TestCase {

    public void testGetBackups() {
        final FtpContext ftpContext = createMock(FtpContext.class);

        String[] directoryNames = new String[] {
            "200804221100", "200804221101", "200804221102"
        };
        String[] names1 = new String[] {
            BackupPlan.CONFIGURATION_ARCHIVE, BackupPlan.VOICEMAIL_ARCHIVE
        };
        String[] names2 = new String[] {
            BackupPlan.CONFIGURATION_ARCHIVE
        };
        String[] names3 = new String[] {
            BackupPlan.VOICEMAIL_ARCHIVE
        };

        ftpContext.openConnection();

        expect(ftpContext.listDirectories(".")).andReturn(directoryNames).once();
        expect(ftpContext.listDirectories(directoryNames[0])).andReturn(EMPTY_STRING_ARRAY);
        expect(ftpContext.listFiles(directoryNames[0])).andReturn(names1).anyTimes();
        expect(ftpContext.listDirectories(directoryNames[1])).andReturn(EMPTY_STRING_ARRAY);
        expect(ftpContext.listFiles(directoryNames[1])).andReturn(names2).anyTimes();
        expect(ftpContext.listDirectories(directoryNames[2])).andReturn(EMPTY_STRING_ARRAY);
        expect(ftpContext.listFiles(directoryNames[2])).andReturn(names3).anyTimes();

        ftpContext.closeConnection();

        replay(ftpContext);

        FtpConfiguration ftpConfiguration = new FtpConfiguration() {
            @Override
            public FtpContext getFtpContext() {
                return ftpContext;
            }
        };

        FtpBackupPlan backupPlan = new FtpBackupPlan();
        backupPlan.setFtpConfiguration(ftpConfiguration);

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

        verify(ftpContext);
    }

}
