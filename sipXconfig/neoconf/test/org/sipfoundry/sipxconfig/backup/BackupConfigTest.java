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
        Collection<Location> hosts = Collections.singleton(new Location("one", "1.1.1.1"));
        BackupPlan ftpPlan = new BackupPlan(BackupType.ftp);
        ftpPlan.setLimitedCount(20);
        StringWriter actual = new StringWriter();
        config.writePrimaryBackupConfig(actual, ftpPlan, hosts, settings);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-cluster-backup.yaml"));        
        assertEquals(expected, actual.toString());        
    }
    
    @Test
    public void schedules() throws IOException {
        BackupConfig config = new BackupConfig();
        DailyBackupSchedule s1 = new DailyBackupSchedule();
        s1.setTimeOfDay(new TimeOfDay(22, 30));
        s1.setScheduledDay(ScheduledDay.FRIDAY);
        DailyBackupSchedule s2 = new DailyBackupSchedule();
        StringWriter actual = new StringWriter();
        config.writeBackupSchedules(actual, BackupType.local, Arrays.asList(s1, s2));
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-schedules.cfdat"));        
        assertEquals(expected, actual.toString());        
    }
    
    @Test
    public void cfengine() throws IOException {
        BackupConfig config = new BackupConfig();
        Collection<String> auto = Arrays.asList("a", "b");
        Collection<String> manual = Arrays.asList("x", "y");
        StringWriter actual = new StringWriter();
        config.writeCfengineConfig(actual, BackupType.local, auto, manual);        
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-cfengine.cfdat"));        
        assertEquals(expected, actual.toString());        
    }
}
