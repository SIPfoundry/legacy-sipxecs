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
import java.io.FilenameFilter;
import java.io.IOException;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.Timer;
import java.util.TimerTask;

import org.apache.commons.io.FileUtils;
import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.mail.MailSenderContext;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;

/**
 * Backup various parts of the system to a fixed backup directory.
 */
public class BackupPlan extends BeanWithId implements ApplicationContextAware {
    public static final String VOICEMAIL_ARCHIVE = "voicemail.tar.gz";
    public static final String CONFIGURATION_ARCHIVE = "configuration.tar.gz";
    public static final FilenameFilter BACKUP_FILE_FILTER = new FilenameFilter() {
        public boolean accept(File dir, String name) {
            return name.equalsIgnoreCase(VOICEMAIL_ARCHIVE)
                    || name.equalsIgnoreCase(CONFIGURATION_ARCHIVE);
        }
    };

    private static final Log LOG = LogFactory.getLog(BackupPlan.class);

    /* ensures we do not get caught in infinite loop */
    private static final int MAX_BACKUPS_TO_DELETE = 100;

    private static final int SUCCESS = 0;

    private String m_backupScript = "sipx-backup";

    private boolean m_voicemail = true;
    private boolean m_configs = true;
    private Integer m_limitedCount;
    private Date m_backupTime;
    private String m_emailAddress;
    
    private ApplicationContext m_applicationContext;
    private String m_emailFromAddress;

    private MailSenderContext m_mailSenderContext;

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
                File[] backupFiles = getBackupFiles(backupDir);
                sendEmail(backupFiles);
                return backupFiles;
            }
        } catch (IOException e) {
            LOG.error(errorMsg, e);
        } catch (InterruptedException e) {
            LOG.error(errorMsg, e);
        }
        return null;
    }

    /**
     * Sends e-mail with a copy of a configuration backup attached. Email is only sent if
     * configuration backup was selected and if e-mail adress is configured.
     * 
     * @param backupFiles array of backup files
     */
    private void sendEmail(File[] backupFiles) {
        if (StringUtils.isBlank(m_emailAddress)) {
            return;
        }
        File confFile = null;
        for (File f : backupFiles) {
            if (f.getName().equals(BackupPlan.CONFIGURATION_ARCHIVE)) {
                confFile = f;
                break;
            }
        }
        if (confFile == null) {
            return;
        }
        Locale locale = Locale.getDefault();
        String subject = m_applicationContext.getMessage("backup.subject", ArrayUtils.EMPTY_OBJECT_ARRAY, locale);
        String body = m_applicationContext.getMessage("backup.body", ArrayUtils.EMPTY_OBJECT_ARRAY, locale);
        m_mailSenderContext.sendMail(m_emailAddress, m_emailFromAddress, subject, body, confFile);
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

    public void setApplicationContext(ApplicationContext applicationContext) {
        m_applicationContext = applicationContext;
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

    public String getEmailAddress() {
        return m_emailAddress;
    }

    public void setEmailAddress(String emailAddress) {
        m_emailAddress = emailAddress;
    }

    public MailSenderContext getMailSenderContext() {
        return m_mailSenderContext;
    }

    public void setMailSenderContext(MailSenderContext mailSenderContext) {
        this.m_mailSenderContext = mailSenderContext;
    }
    
    public void setEmailFromAddress(String emailFromAddress) {
        m_emailFromAddress = emailFromAddress;
    }
}
