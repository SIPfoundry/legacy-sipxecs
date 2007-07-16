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
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Date;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import org.apache.commons.io.FileUtils;
import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.BeanWithId;

/**
 * Backup various parts of the system to a fixed backup directory.
 */
public class BackupPlan extends BeanWithId {

    private static final Log LOG = LogFactory.getLog(BackupPlan.class);

    private static final String VOICEMAIL_ARCHIVE = "voicemail.tar.gz";
    private static final String CONFIGURATION_ARCHIVE = "configuration.tar.gz";

    /* ensures we do not get caught in infinite loop */
    private static final int MAX_BACKUPS_TO_DELETE = 100;

    private static final int SUCCESS = 0;

    private String m_backupScript = "sipx-backup";

    private boolean m_voicemail = true;
    private boolean m_configs = true;
    private Integer m_limitedCount;
    private Date m_backupTime;

    private Collection<DailyBackupSchedule> m_schedules = new ArrayList<DailyBackupSchedule>(0);

    public File[] perform(String rootBackupPath, String binPath) {
        String errorMsg = "Errors when creating backup.";
        try {
            File rootBackupDir = new File(rootBackupPath);
            File backupDir = getNextBackupDir(rootBackupDir);
            if (!backupDir.isDirectory()) {
                backupDir.mkdirs();
            }
            File binDir = new File(binPath);
            if (SUCCESS == perform(backupDir, binDir)) {
                return getBackupFiles(backupDir);
            }
        } catch (IOException e) {
            LOG.error(errorMsg, e);
        } catch (InterruptedException e) {
            LOG.error(errorMsg, e);
        }
        return null;
    }

    File getNextBackupDir(File rootBackupDir) {
        m_backupTime = new Date();
        DateFormat fmt = new SimpleDateFormat("yyyyMMddHHmm");
        File nextDir = new File(rootBackupDir, fmt.format(m_backupTime));

        String purgeable;
        int i = 0;
        do {
            purgeable = getOldestPurgableBackup(rootBackupDir.list());
            if (purgeable != null) {
                try {
                    File oldBackup = new File(rootBackupDir, purgeable);
                    LOG.info(String.format("Deleting old backup '%s'", oldBackup
                            .getAbsolutePath()));
                    FileUtils.deleteDirectory(oldBackup);
                    if (i++ > MAX_BACKUPS_TO_DELETE) {
                        LOG.error("Avoiding infinite loop trying to remove old backups");
                        break;
                    }
                } catch (IOException nonfatal) {
                    LOG.error("Could not limit backup count", nonfatal);
                    break;
                }
            }
        } while (purgeable != null);

        return nextDir;
    }

    String getOldestPurgableBackup(String[] filelist) {
        if (m_limitedCount == null || filelist == null) {
            return null;
        }

        if (filelist.length < m_limitedCount) {
            return null;
        }

        Arrays.sort(filelist);
        return filelist[0];
    }

    void setScript(String script) {
        m_backupScript = script;
    }

    private int perform(File workingDir, File binDir) throws IOException, InterruptedException {
        String cmdLine = new String(binDir.getPath() + File.separator + m_backupScript + " -n");
        if (!isVoicemail()) {
            // Configuration only.
            cmdLine += " -c";
        } else if (!isConfigs()) {
            // Voicemail only.
            cmdLine += " -v";
        }

        Process process = Runtime.getRuntime().exec(cmdLine, ArrayUtils.EMPTY_STRING_ARRAY,
                workingDir);
        int code = process.waitFor();
        if (SUCCESS != code) {
            String errorMsg = String.format("Backup operation failed. Exit code: %d", code);
            LOG.error(errorMsg);
        }

        return code;
    }

    File[] getBackupFiles(File backupDir) {
        List files = new ArrayList();
        if (isConfigs()) {
            File configuration = new File(backupDir, CONFIGURATION_ARCHIVE);
            files.add(configuration);
        }
        if (isVoicemail()) {
            File voicemail = new File(backupDir, VOICEMAIL_ARCHIVE);
            files.add(voicemail);
        }
        return (File[]) files.toArray(new File[files.size()]);
    }

    public void addSchedule(DailyBackupSchedule dailySchedule) {
        m_schedules.add(dailySchedule);
        dailySchedule.setBackupPlan(this);
    }

    public Collection<DailyBackupSchedule> getSchedules() {
        return m_schedules;
    }

    public void setSchedules(Collection<DailyBackupSchedule> schedules) {
        m_schedules = schedules;
    }

    public Integer getLimitedCount() {
        return m_limitedCount;
    }

    public void setLimitedCount(Integer limitedCount) {
        m_limitedCount = limitedCount;
    }

    public boolean isConfigs() {
        return m_configs;
    }

    public void setConfigs(boolean configs) {
        m_configs = configs;
    }

    public boolean isVoicemail() {
        return m_voicemail;
    }

    public void setVoicemail(boolean voicemail) {
        m_voicemail = voicemail;
    }

    public void schedule(Timer timer, String rootBackupPath, String binPath) {
        schedule(timer, getTask(rootBackupPath, binPath));
    }

    void schedule(Timer timer, TimerTask task) {
        for (DailyBackupSchedule schedule : getSchedules()) {
            schedule.schedule(timer, task);
        }
    }

    TimerTask getTask(String rootBackupPath, String binPath) {
        return new BackupTask(rootBackupPath, binPath);
    }

    class BackupTask extends TimerTask {
        private String m_rootBackupPath;

        private String m_binPath;

        BackupTask(String rootBackupPath, String binPath) {
            m_rootBackupPath = rootBackupPath;
            m_binPath = binPath;
        }

        public void run() {
            BackupPlan.this.perform(m_rootBackupPath, m_binPath);
        }
    }
}
