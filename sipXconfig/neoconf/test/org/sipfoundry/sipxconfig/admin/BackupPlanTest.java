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
import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;

import junit.framework.TestCase;

import org.easymock.IMocksControl;
import org.easymock.classextension.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class BackupPlanTest extends TestCase {

    private BackupPlan m_backup;
    
    protected void setUp() throws Exception {
        m_backup = new BackupPlan();
        m_backup.setConfigsScript("mock-backup.sh");
        m_backup.setMailstoreScript("mock-backup.sh");
    }

    public void testBuildExecName() {
        File tmp = new File("/tmp");
        String cmdLine = m_backup.buildExecName(tmp, "kuku");
        if (TestUtil.isWindows()) {
            assertEquals(":\\tmp\\kuku --non-interactive", cmdLine.substring(1));            
        } else {
            assertEquals("/tmp/kuku --non-interactive", cmdLine);
        }
    }

    public void testGetBackupLocations() {
        File[] backupLocations = m_backup.getBackupFiles(new File("."));
        assertEquals(3, backupLocations.length);
        String[] refBackupLocations = {
            "./backup-configs/fs.tar.gz", "./backup-configs/pds.tar.gz",
            "./backup-mailstore/mailstore.tar.gz"
        };
        for (int i = 0; i < refBackupLocations.length; i++) {
            String expected = refBackupLocations[i].replace('/', File.separatorChar); 
            assertEquals(expected, backupLocations[i].getPath());
        }
    }

    public void testPerform() throws Exception {
        if (TestUtil.isWindows()) {
            // tries to run a shell script. ignore test
            return;
        }
        String backupPath = TestHelper.getTestDirectory() + "/backup-" + System.currentTimeMillis();
        File[] backups = m_backup.perform(backupPath, TestUtil.getTestSourceDirectory(this.getClass()));
        assertEquals(3, backups.length);
        assertTrue(backups[0].exists());
        assertTrue(backups[1].exists());
        assertTrue(backups[2].exists());
    }
    
    public void testGetNextBackupDir() {        
        File f = m_backup.getNextBackupDir(new File("."));
        assertTrue(f.getName().matches("\\d{12}"));
    }
    
    public void testOldestPurgableBackup() throws Exception {
        m_backup.setLimitedCount(new Integer(2));
        assertNull(m_backup.getOldestPurgableBackup(null));
        assertNull(m_backup.getOldestPurgableBackup(new String[0]));
        assertNull(m_backup.getOldestPurgableBackup(new String[] { "20050501" }));
        assertEquals("20050501", m_backup.getOldestPurgableBackup(new String[] { "20050501", "20050502"}));        
    }
    
    public void testDailyTimer() throws Exception {
        checkTimer(ScheduledDay.EVERYDAY, DailyBackupSchedule.ONCE_A_DAY);
    }
    
    public void testWeeklyTimer() throws Exception {
        checkTimer(ScheduledDay.SUNDAY, DailyBackupSchedule.ONCE_A_WEEK);
    }
    
    private void checkTimer(ScheduledDay day, long timerPeriod) {
        BackupPlan plan = new BackupPlan();
        DailyBackupSchedule schedule = new DailyBackupSchedule();
        schedule.setEnabled(true);
        schedule.setScheduledDay(day);
        plan.addSchedule(schedule);
        TimerTask task = plan.getTask("root", "bin");
        
        IMocksControl timerControl = EasyMock.createStrictControl();
        Timer timer = timerControl.createMock(Timer.class);
        
        Date d = schedule.getTimerDate();
        assertTrue(d.getTime() > System.currentTimeMillis());   // must be scheduled in the future
        timer.schedule(task, d, timerPeriod);
        timerControl.replay();

        plan.schedule(timer, task);
        
        timerControl.verify();        
    }
}
