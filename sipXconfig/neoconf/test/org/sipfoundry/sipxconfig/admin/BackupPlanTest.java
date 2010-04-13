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

import org.apache.commons.io.FileUtils;
import org.apache.commons.lang.ArrayUtils;
import org.easymock.IMocksControl;
import org.easymock.classextension.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.mail.MailSenderContextImpl;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class BackupPlanTest extends TestCase {

    private BackupPlan m_backup;

    @Override
    protected void setUp() throws Exception {
        m_backup = new LocalBackupPlan();
        m_backup.setMailSenderContext(new MailSenderContextImpl());
        m_backup.setScript("mock-backup.sh");
        String backupPath = TestHelper.getTestDirectory() + File.separator + System.currentTimeMillis();
        m_backup.setBackupDirectory(backupPath);
    }

    public void testGetBackupLocations() {
        File[] backupLocations = m_backup.getBackupFiles(new File("."));
        assertEquals(2, backupLocations.length);
        String[] refBackupLocations = {
            "./configuration.tar.gz", "./voicemail.tar.gz"
        };
        for (int i = 0; i < refBackupLocations.length; i++) {
            String expected = refBackupLocations[i].replace('/', File.separatorChar);
            assertEquals(expected, backupLocations[i].getPath());
        }
    }

    public void testPurgeOld() throws Exception {
        TestHelper.getTestDirectory();
        File root = new File(TestHelper.getTestDirectory(), "purge-" + System.currentTimeMillis());
        for (int i = 0; i < 5; i++) {
            File testDir = new File(root, "dir-" + i);
            boolean mkdirs = testDir.mkdirs();
            assertTrue(mkdirs);
        }
        assertEquals(5, root.list().length);
        m_backup.setLimitedCount(5);
        m_backup.setBackupDirectory(root.getPath());
        m_backup.purgeOld();
        assertEquals(5, root.list().length);
        m_backup.setLimitedCount(2);
        m_backup.purgeOld();
        String[] list = root.list();
        assertEquals(2, list.length);
        assertTrue(ArrayUtils.contains(list, "dir-3"));
        assertTrue(ArrayUtils.contains(list, "dir-4"));
        m_backup.setLimitedCount(0);
        m_backup.purgeOld();
        list = root.list();
        assertEquals(1, list.length);
        assertEquals("dir-4", list[0]);
        FileUtils.deleteDirectory(root);
    }

    public void testPerform() throws Exception {
        if (TestUtil.isWindows()) {
            // tries to run a shell script. ignore test
            return;
        }
        File[] backups = m_backup.doPerform(TestUtil.getTestSourceDirectory(this.getClass()));
        assertEquals(2, backups.length);
        assertTrue(backups[0].exists());
        assertTrue(backups[1].exists());
    }

    public void testGetNextBackupDir() {
        File f = m_backup.getNextBackupDir(new File("."));
        assertTrue(f.getName().matches("\\d{12}"));
    }

    public void testDailyTimer() throws Exception {
        checkTimer(ScheduledDay.EVERYDAY, DailyBackupSchedule.ONCE_A_DAY);
    }

    public void testWeeklyTimer() throws Exception {
        checkTimer(ScheduledDay.SUNDAY, DailyBackupSchedule.ONCE_A_WEEK);
    }

    private void checkTimer(ScheduledDay day, long timerPeriod) {
        BackupPlan plan = new LocalBackupPlan();
        DailyBackupSchedule schedule = new DailyBackupSchedule();
        schedule.setEnabled(true);
        schedule.setScheduledDay(day);
        plan.addSchedule(schedule);
        TimerTask task = plan.getTask("bin");

        IMocksControl timerControl = EasyMock.createStrictControl();
        Timer timer = timerControl.createMock(Timer.class);

        Date d = schedule.getTimerDate();
        assertTrue(d.getTime() > System.currentTimeMillis()); // must be scheduled in the future
        timer.schedule(task, d, timerPeriod);
        timerControl.replay();

        plan.schedule(timer, task);

        timerControl.verify();
    }
}
