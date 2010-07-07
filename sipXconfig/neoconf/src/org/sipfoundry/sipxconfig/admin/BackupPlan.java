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
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.BackupBean.Type;
import org.sipfoundry.sipxconfig.admin.mail.MailSenderContext;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;

/**
 * Backup various parts of the system to a fixed backup directory.
 */
public abstract class BackupPlan extends BeanWithId implements ApplicationContextAware {
    public static final String VOICEMAIL_ARCHIVE = "voicemail.tar.gz";
    public static final String CONFIGURATION_ARCHIVE = "configuration.tar.gz";
    public static final FilenameFilter BACKUP_FILE_FILTER = new FilenameFilter() {
        public boolean accept(File dir, String name) {
            return name.equalsIgnoreCase(VOICEMAIL_ARCHIVE) || name.equalsIgnoreCase(CONFIGURATION_ARCHIVE);
        }
    };
    private static final SimpleDateFormat FILE_NAME_FORMAT = new SimpleDateFormat("yyyyMMddHHmm");

    private static final Log LOG = LogFactory.getLog(BackupPlan.class);

    private String m_backupScript = "sipx-backup";

    private boolean m_voicemail = true;
    private boolean m_configs = true;
    private Integer m_limitedCount = 50;
    private Date m_backupTime;
    private String m_emailAddress;

    private ApplicationContext m_applicationContext;
    private String m_emailFromAddress;

    private MailSenderContext m_mailSenderContext;

    private Collection<DailyBackupSchedule> m_schedules = new ArrayList<DailyBackupSchedule>(0);

    private Timer m_timer;

    private String m_backupDirectory;

    private Locale m_locale;

    public abstract List<Map<Type, BackupBean>> getBackups();

    public abstract File[] doPerform(String binPath) throws IOException, InterruptedException;

    protected abstract void doPurge(int limitCount);

    /**
     * Purge old files and perform backup.
     *
     * We are purging old backup twice here: the first round is in most cases no-op: it only
     * matters if administrator decreased the number of saved backups to free up some disk space.
     * The second round, after successful completion of the current backup will remove a single
     * oldest backup.
     *
     * @param binPath path to the script that performs the backup
     * @return array of files that were saved/created by this backup plan
     */
    public final File[] perform(String binPath) {
        String errorMsg = "Errors when creating backup.";
        try {
            purgeOld();
            File[] files = doPerform(binPath);
            purgeOld();
            return files;
        } catch (IOException e) {
            LOG.error(errorMsg, e);
        } catch (InterruptedException e) {
            LOG.error(errorMsg, e);
        }
        return null;
    }

    protected final File createBackupDirectory(File rootBackupDir) {
        File backupDir = getNextBackupDir(rootBackupDir);
        if (!backupDir.isDirectory()) {
            backupDir.mkdirs();
        }
        return backupDir;
    }

    protected final File[] executeBackup(File backupDir, File binDir) throws IOException, InterruptedException {
        if (perform(backupDir, binDir)) {
            File[] backupFiles = getBackupFiles(backupDir);
            sendEmail(backupFiles);
            return backupFiles;
        }
        return null;
    }

    protected final void purgeOld() {
        if (m_limitedCount == null) {
            return;
        }
        if (m_limitedCount < 1) {
            // have to leave at least on
            m_limitedCount = 1;
        }
        doPurge(m_limitedCount);
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
        String subject = m_applicationContext.getMessage("backup.subject", ArrayUtils.EMPTY_OBJECT_ARRAY, m_locale);
        String body = m_applicationContext.getMessage("backup.body", ArrayUtils.EMPTY_OBJECT_ARRAY, m_locale);
        m_mailSenderContext.sendMail(m_emailAddress, m_emailFromAddress, subject, body, confFile);
    }

    File getNextBackupDir(File rootBackupDir) {
        // FIXME: only works if no more than one backup a minute
        m_backupTime = new Date();
        return new File(rootBackupDir, FILE_NAME_FORMAT.format(m_backupTime));
    }

    void setScript(String script) {
        m_backupScript = script;
    }

    private boolean perform(File workingDir, File binDir) throws IOException, InterruptedException {
        ProcessBuilder pb = new ProcessBuilder(binDir.getPath() + File.separator + m_backupScript, "-n");
        if (!isVoicemail()) {
            // Configuration only.
            pb.command().add("-c");
        } else if (!isConfigs()) {
            // Voicemail only.
            pb.command().add("-v");
        }

        Process process = pb.directory(workingDir).start();
        int code = process.waitFor();
        if (code != 0) {
            String errorMsg = String.format("Backup operation failed. Exit code: %d", code);
            LOG.error(errorMsg);
            return false;
        }

        return true;
    }

    /**
     * create and add BackupBeans
     *
     * @param repositoryBean
     * @param backupFiles
     */
    protected void addBackupBeans(List<Map<Type, BackupBean>> repositoryBean, File[] backupFiles) {
        Map<Type, BackupBean> backups = new HashMap<Type, BackupBean>(2);
        for (File file : backupFiles) {
            BackupBean backupBean = new BackupBean(file);
            backups.put(backupBean.getType(), backupBean);

        }
        if (!backups.isEmpty()) {
            repositoryBean.add(backups);
        }
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

    /**
     * For backup to make sense at least one of the parameters (i.e. voicemail or configuration)
     * have to be set.
     */
    public boolean isEmpty() {
        return !(m_voicemail || m_configs);
    }

    public void schedule(Timer timer, String binPath) {
        schedule(timer, getTask(binPath));
    }

    void schedule(Timer timer, TimerTask task) {
        for (DailyBackupSchedule schedule : getSchedules()) {
            schedule.schedule(timer, task);
        }
    }

    TimerTask getTask(String binPath) {
        return new BackupTask(binPath);
    }

    class BackupTask extends TimerTask {
        private final String m_binPath;

        BackupTask(String binPath) {
            m_binPath = binPath;
        }

        @Override
        public void run() {
            BackupPlan.this.perform(m_binPath);
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
        m_mailSenderContext = mailSenderContext;
    }

    public void setEmailFromAddress(String emailFromAddress) {
        m_emailFromAddress = emailFromAddress;
    }

    public Timer getTimer() {
        return m_timer;
    }

    public void setTimer(Timer timer) {
        m_timer = timer;
    }

    @Required
    public void setBackupDirectory(String backupDirectory) {
        m_backupDirectory = backupDirectory;
    }

    protected String getBackupDirectory() {
        return m_backupDirectory;
    }

    public void resetTimer(String binDirectory) {
        Timer timer = getTimer();
        if (timer != null) {
            timer.cancel();
        }
        timer = new Timer(false); // daemon, dies with main thread
        setTimer(timer);
        schedule(timer, binDirectory);
    }

    public void setLocale(Locale locale) {
        this.m_locale = locale;
    }

    public Locale getLocale() {
        return m_locale;
    }
}
