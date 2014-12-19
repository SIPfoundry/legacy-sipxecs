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
import java.util.TreeSet;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.junit.Test;
import org.sipfoundry.commons.security.Util;
import org.sipfoundry.sipxconfig.cfgmgt.YamlConfiguration;
import org.sipfoundry.sipxconfig.common.ScheduledDay;
import org.sipfoundry.sipxconfig.common.TimeOfDay;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class BackupConfigTest {

    @Test
    public void hostConfig() throws IOException {
        BackupConfig config = new BackupConfig();
        ArchiveDefinition d1 = new ArchiveDefinition("d1", "b1", "r1");
        ArchiveDefinition d2 = new ArchiveDefinition("d2", "b2", "r2");
        ArchiveDefinition d3 = new ArchiveDefinition("d3", "b3", "r3");
        StringWriter actual = new StringWriter();
        Location l1 = new Location("one", "1.1.1.1");
        l1.setUniqueId(1);
        BackupConfig.BackupRestore backupRestore = new BackupConfig.BackupRestore();
        backupRestore.setBackup(true);
        backupRestore.setRestore(true);
        config.writeHostDefinitions(new YamlConfiguration(actual), l1, Arrays.asList(d1, d2, d3),
            backupRestore, new TreeSet<ArchiveDefinition>(), new TreeSet<ArchiveDefinition>());
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
        Location l2 = new Location("two", "2.2.2.2");
        l2.setUniqueId(2);
        Collection<Location> hosts = Arrays.asList(l1, l2);

        ArchiveDefinition d1 = new ArchiveDefinition("d1", "backup", "restore");
        ArchiveDefinition d2 = new ArchiveDefinition("d2", "backup", "restore");
        ArchiveDefinition d3 = new ArchiveDefinition("d3", "backup", "restore");

        BackupPlan plan = new BackupPlan(BackupType.ftp);
        plan.setDefinitionIds(new TreeSet<String>(Arrays.asList("d1", "d2", "d3")));
        plan.setLimitedCount(20);

        BackupManager mgr = createMock(BackupManager.class);
        mgr.getArchiveDefinitions(l1, null, null);
        expectLastCall().andReturn(Arrays.asList(d1, d2)).anyTimes();
        mgr.getArchiveDefinitions(l2, null, null);
        expectLastCall().andReturn(Arrays.asList(d3)).anyTimes();
        mgr.getArchiveDefinitions(l1, plan, settings);
        expectLastCall().andReturn(Arrays.asList(d1, d2)).anyTimes();
        mgr.getArchiveDefinitions(l2, plan, settings);
        expectLastCall().andReturn(Arrays.asList(d3)).anyTimes();
        replay(mgr);
        config.setBackupManager(mgr);

        StringWriter actual = new StringWriter();
        config.writeConfig(actual, plan, hosts, settings);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-auto-backup.yaml"));
        assertEquals(expected, actual.toString());

        verify(mgr);
    }
    
    @Test
    public void clusterRedundantSingleNodeBackup() throws IOException {
        BackupConfig config = new BackupConfig();

        BackupSettings settings = new BackupSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        settings.setSettingTypedValue("ftp/url", "ftp://ftp.example.org");
        settings.setSettingTypedValue("ftp/user", "joe");
        settings.setSettingTypedValue("ftp/password", "xxx");

        Location l1 = new Location("one", "1.1.1.1");
        l1.setUniqueId(1);
        Location l2 = new Location("two", "2.2.2.2");
        l2.setUniqueId(2);
        Collection<Location> hosts = Arrays.asList(l1, l2);

        ArchiveDefinition d1 = new ArchiveDefinition("d1", "backup", "restore");
        ArchiveDefinition d2 = new ArchiveDefinition("d2", "backup", "restore");
        ArchiveDefinition d3 = new ArchiveDefinition("d3", "backup", "restore");

        BackupPlan plan = new BackupPlan(BackupType.ftp);
        plan.setDefinitionIds(new TreeSet<String>(Arrays.asList("d1", "d2", "d3")));
        plan.setLimitedCount(20);

        BackupManager mgr = createMock(BackupManager.class);
        mgr.getArchiveDefinitions(l1, null, null);
        expectLastCall().andReturn(Arrays.asList(d1, d2)).anyTimes();
        mgr.getArchiveDefinitions(l2, null, null);
        expectLastCall().andReturn(Arrays.asList(d2, d3)).anyTimes();
        mgr.getArchiveDefinitions(l1, plan, settings);
        expectLastCall().andReturn(Arrays.asList(d1, d2)).anyTimes();
        mgr.getArchiveDefinitions(l2, plan, settings);
        expectLastCall().andReturn(Arrays.asList(d2, d3)).anyTimes();
        replay(mgr);
        config.setBackupManager(mgr);

        StringWriter actual = new StringWriter();
        config.writeConfig(actual, plan, hosts, settings);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-auto-backup.yaml"));
        assertEquals(expected, actual.toString());

        verify(mgr);
    }    

    @Test
    public void clusterBackupRestoreOnDifferentNodes() throws IOException {
        BackupConfig config = new BackupConfig();

        BackupSettings settings = new BackupSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        settings.setSettingTypedValue("ftp/url", "ftp://ftp.example.org");
        settings.setSettingTypedValue("ftp/user", "joe");
        settings.setSettingTypedValue("ftp/password", "xxx");

        Location l1 = new Location("one", "1.1.1.1");
        l1.setUniqueId(1);
        Location l2 = new Location("two", "2.2.2.2");
        l2.setUniqueId(2);
        Collection<Location> hosts = Arrays.asList(l1, l2);

        ArchiveDefinition d1 = new ArchiveDefinition("archive.tar.gz", "backup", null);
        ArchiveDefinition d2 = new ArchiveDefinition("archive.tar.gz", null, "restore");

        BackupPlan plan = new BackupPlan(BackupType.ftp);
        plan.setDefinitionIds(new TreeSet<String>(Arrays.asList("archive.tar.gz")));
        plan.setLimitedCount(20);

        BackupManager mgr = createMock(BackupManager.class);
        mgr.getArchiveDefinitions(l1, null, null);
        expectLastCall().andReturn(Arrays.asList(d1)).anyTimes();
        mgr.getArchiveDefinitions(l2, null, null);
        expectLastCall().andReturn(Arrays.asList(d2)).anyTimes();
        mgr.getArchiveDefinitions(l1, plan, settings);
        expectLastCall().andReturn(Arrays.asList(d1)).anyTimes();
        mgr.getArchiveDefinitions(l2, plan, settings);
        expectLastCall().andReturn(Arrays.asList(d2)).anyTimes();
        replay(mgr);

        config.setBackupManager(mgr);
        StringWriter actual = new StringWriter();
        config.writeConfig(actual, plan, hosts, settings);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-different-nodes-backup.yaml"));
        assertEquals(expected, actual.toString());

        verify(mgr);
    }

    @Test
    public void clusterBackupOnSecondaryRestoreOnPrimary() throws IOException {
        BackupConfig config = new BackupConfig();

        BackupSettings settings = new BackupSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());

        Location l1 = new Location("one", "1.1.1.1");
        l1.setUniqueId(1);
        l1.setPrimary(true);
        Location l2 = new Location("two", "2.2.2.2");
        l2.setUniqueId(2);
        Collection<Location> hosts = Arrays.asList(l1, l2);

        //3 possible archive definitions . 'archive' to backup on secondary, restore on primary
        //others backup/restore on primary
        ArchiveDefinition d1 = new ArchiveDefinition("archive.tar.gz", "backup", null);
        ArchiveDefinition d2 = new ArchiveDefinition("archive.tar.gz", null, "restore");
        ArchiveDefinition d3 = new ArchiveDefinition("archive2.tar.gz", "backup", "restore");
        ArchiveDefinition d4 = new ArchiveDefinition("archive3.tar.gz", "backup", "restore");

        BackupPlan plan = new BackupPlan(BackupType.local);
        //select only one definition to backup or restore
        plan.setDefinitionIds(new TreeSet<String>(Arrays.asList("archive.tar.gz")));
        plan.setLimitedCount(20);

        BackupManager mgr = createMock(BackupManager.class);

        mgr.getArchiveDefinitions(l1, plan, settings);
        expectLastCall().andReturn(Arrays.asList(d2, d3, d4)).anyTimes();
        mgr.getArchiveDefinitions(l2, plan, settings);
        expectLastCall().andReturn(Arrays.asList(d1, d3, d4)).anyTimes();
        replay(mgr);

        config.setBackupManager(mgr);
        StringWriter actual = new StringWriter();
        config.writeConfig(actual, plan, hosts, settings);
        String expected = IOUtils.toString(getClass().getResourceAsStream("backup-on-secondary-restore-on-primary.yaml"));
        assertEquals(expected, actual.toString());

        verify(mgr);
    }

    @Test
    public void testHexUnicodePassword() {
        String[] password = Util.hexUnicodeEscape("#123");
        String passwToSave = StringUtils.join(password, '.');
        assertEquals("35.49.50.51", passwToSave);
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
