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
import java.io.FileFilter;
import java.util.ArrayList;
import java.util.List;
import java.util.Timer;

import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

/**
 * Backup provides Java interface to backup scripts
 */
public class AdminContextImpl extends HibernateDaoSupport implements AdminContext,
        ApplicationListener {

    private String m_binDirectory;

    private String m_backupDirectory;

    private Timer m_timer;

    public String getBackupDirectory() {
        return m_backupDirectory;
    }

    public void setBackupDirectory(String backupDirectory) {
        m_backupDirectory = backupDirectory;
    }

    public String getBinDirectory() {
        return m_binDirectory;
    }

    public void setBinDirectory(String binDirectory) {
        m_binDirectory = binDirectory;
    }

    public BackupPlan getBackupPlan() {
        List plans = getHibernateTemplate().loadAll(BackupPlan.class);
        BackupPlan plan = (BackupPlan) DaoUtils.requireOneOrZero(plans, "all backup plans");

        // create a new one if one doesn't exists, otherwise
        // risk having 2 or more in database
        if (plan == null) {
            plan = new BackupPlan();
            storeBackupPlan(plan);
        }
        return plan;
    }

    public void storeBackupPlan(BackupPlan plan) {
        getHibernateTemplate().saveOrUpdate(plan);
        resetTimer(plan);
    }

    public File[] performBackup(BackupPlan plan) {
        return plan.perform(m_backupDirectory, m_binDirectory);
    }

    /**
     * start backup timers after app is initialized
     */
    public void onApplicationEvent(ApplicationEvent event) {
        // No need to register listener, all beans that implement listener
        // interface are
        // automatically registered
        if (event instanceof ApplicationInitializedEvent) {
            resetTimer(getBackupPlan());
        }
    }

    private void resetTimer(BackupPlan plan) {
        if (m_timer != null) {
            m_timer.cancel();
        }
        m_timer = new Timer(false); // deamon, dies with main thread
        plan.schedule(m_timer, m_backupDirectory, m_binDirectory);
    }

    public String[] getInitializationTasks() {
        List l = getHibernateTemplate().findByNamedQuery("taskNames");
        return (String[]) l.toArray(new String[l.size()]);
    }

    public void deleteInitializationTask(String task) {
        List l = getHibernateTemplate().findByNamedQueryAndNamedParam("taskByName", "task", task);
        getHibernateTemplate().deleteAll(l);
    }

    public BackupBean[] getBackups() {
        final String configurationName = "Configuration";
        final String voicemailName = "Voicemail";
        List<BackupBean> backupList = new ArrayList<BackupBean>();
        File backupDirectory = new File(getBackupDirectory());
        File[] backupFolders = backupDirectory.listFiles();

        for (File backupFolder : backupFolders) {
            if (backupFolder.isDirectory()) {
                File[] backups = backupFolder.listFiles(getBackupFileFilter());
                String backupName = null;
                for (File backup : backups) {
                    if (backup.getName().equalsIgnoreCase(Restore.BACKUP_CONFIGS)) {
                        backupName = configurationName;
                    } else {
                        backupName = voicemailName;
                    }
                    backupList.add(new BackupBean(backupName, backupFolder.getName(), backup
                            .getAbsolutePath()));
                }
            }
        }

        return backupList.toArray(new BackupBean[backupList.size()]);
    }

    private FileFilter getBackupFileFilter() {
        FileFilter backupFileFilter = new FileFilter() {
            public boolean accept(File file) {
                return file.getName().equalsIgnoreCase(Restore.BACKUP_CONFIGS)
                        || file.getName().equalsIgnoreCase(Restore.BACKUP_MAILSTORE);
            }
        };
        return backupFileFilter;
    }
}
