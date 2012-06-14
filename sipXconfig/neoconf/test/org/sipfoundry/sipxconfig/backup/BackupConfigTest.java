/**
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.backup;


import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.common.ScheduledDay;
import org.sipfoundry.sipxconfig.common.TimeOfDay;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class BackupConfigTest {

    @Test
    public void config() throws IOException {
        BackupConfig config = new BackupConfig();
        ArchiveDefinition d1 = new ArchiveDefinition("d1", "b1", "r1");
        ArchiveDefinition d2 = new ArchiveDefinition("d2", "b2", "r2");
        ArchiveDefinition d3 = new ArchiveDefinition("d3", "b3", "r3");
        StringWriter actual = new StringWriter();
        config.writeBackupDefinitions(actual, Arrays.asList(d1, d2, d3), Arrays.asList("d1", "d2"),
                Arrays.asList("d2", "d3"));
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-backup.yaml"));
        assertEquals(expected, actual.toString());
    }
    
    @Test
    public void cluster() throws IOException {
        BackupConfig config = new BackupConfig();
        
        BackupSettings settings = new BackupSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        settings.setSettingTypedValue("ftp/url", "ftp://ftp.example.org");
        settings.setSettingTypedValue("ftp/user", "joe");
        settings.setSettingTypedValue("ftp/password", "xxx");
        
        Location l1 = new Location("one", "1.1.1.1");
        l1.setUniqueId(1);
        Location l2 = new Location("one", "2.2.2.2");
        l2.setUniqueId(2);
        Collection<Location> hosts = Arrays.asList(l1, l2);
        
        ArchiveDefinition d1 = new ArchiveDefinition("d1", "backup", "restore");
        ArchiveDefinition d2 = new ArchiveDefinition("d2", "backup", "restore");
        
        BackupManager mgr = createMock(BackupManager.class);
        mgr.getArchiveDefinitions(l1, null);
        expectLastCall().andReturn(Collections.singleton(d1)).anyTimes();
        mgr.getArchiveDefinitions(l2, null);
        expectLastCall().andReturn(Collections.singleton(d2)).anyTimes();        
        replay(mgr);
        config.setBackupManager(mgr);
        
        BackupPlan ftpPlan = new BackupPlan(BackupType.ftp);
        ftpPlan.setLimitedCount(20);
        
        StringWriter actual = new StringWriter();
        config.writePrimaryBackupConfig(actual, ftpPlan, null, hosts, settings, null);        
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-auto-backup.yaml"));        
        verify(mgr);
        assertEquals(expected, actual.toString());
        
        mgr = createMock(BackupManager.class);
        mgr.getArchiveDefinitions(l1, null);
        expectLastCall().andReturn(Arrays.asList(d1, d2));
        mgr.getArchiveDefinitions(l2, null);
        expectLastCall().andReturn(Collections.emptyList());        
        replay(mgr);
        config.setBackupManager(mgr);

        BackupSettings manual = new BackupSettings();
        manual.setModelFilesContext(TestHelper.getModelFilesContext());
        manual.setSettingTypedValue("ftp/url", "sftp://sftp.example.org");
        manual.setSettingTypedValue("ftp/user", "mary");
        manual.setSettingTypedValue("ftp/password", "yyy");       
        manual.setSettingTypedValue("restore/resetPin", "555");
        
        actual = new StringWriter();
        config.writePrimaryBackupConfig(actual, ftpPlan, ftpPlan, hosts, settings, manual);
        expected = IOUtils.toString(getClass().getResourceAsStream("expected-manual-backup.yaml"));
        assertEquals(expected, actual.toString());        
    }
    
    @Test
    public void schedules() throws IOException {
        BackupConfig config = new BackupConfig();
        DailyBackupSchedule s1 = new DailyBackupSchedule();
        s1.setTimeOfDay(new TimeOfDay(22, 30));
        s1.setScheduledDay(ScheduledDay.FRIDAY);
        DailyBackupSchedule s2 = new DailyBackupSchedule();
        DailyBackupSchedule s3 = new DailyBackupSchedule();
        s1.setEnabled(true);
        s2.setEnabled(true);
        StringWriter actual = new StringWriter();
        config.writeBackupSchedules(actual, BackupType.local, Arrays.asList(s1, s2, s3));
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-schedules.cfdat"));        
        assertEquals(expected, actual.toString());        
    }
}
