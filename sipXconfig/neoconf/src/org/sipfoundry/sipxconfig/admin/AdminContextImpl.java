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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileFilter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Writer;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Timer;

import org.apache.commons.io.filefilter.DirectoryFileFilter;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.BackupBean.Type;
import org.sipfoundry.sipxconfig.admin.ftp.FtpConfiguration;
import org.sipfoundry.sipxconfig.admin.ftp.FtpContext;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.DSTChangeEvent;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

/**
 * Backup provides Java interface to backup scripts
 */
public class AdminContextImpl extends HibernateDaoSupport implements AdminContext,
        ApplicationListener, BeanFactoryAware {
    private static final String DATE_BINARY = "sipx-sudo-date";
    
    private static final SimpleDateFormat CHANGE_DATE_FORMAT = new SimpleDateFormat("MMddHHmmyyyy");

    private static final Log LOG = LogFactory.getLog(AdminContextImpl.class);

    private String m_binDirectory;
    
    private String m_libExecDirectory;

    private String m_backupDirectory;

    private BeanFactory m_beanFactory;

    private ExportCsv m_exportCsv;

    private String m_ftpBackupDirectory;

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
    
    public String getLibExecDirectory() {
        return m_libExecDirectory;
    }

    public void setLibExecDirectory(String libExecDirectory) {
        m_libExecDirectory = libExecDirectory;
    }

    public void setExportCsv(ExportCsv exportCsv) {
        m_exportCsv = exportCsv;
    }

    public FtpConfiguration getFtpConfiguration() {
        List configurations = getHibernateTemplate().loadAll(FtpConfiguration.class);
        FtpConfiguration configuration = (FtpConfiguration)
            DaoUtils.requireOneOrZero(configurations, "all configurations");
        if (configuration != null) {
            FtpContext ftpContext = ((FtpBackupPlan) configuration.getBackupPlan()).getFtpContext();
            ftpContext.setHost(configuration.getHost());
            ftpContext.setUserId(configuration.getUserId());
            ftpContext.setPassword(configuration.getPassword());
        } else {
            configuration = (FtpConfiguration) m_beanFactory.getBean("ftpConfiguration");
            storeBackupPlan(configuration.getBackupPlan());
            storeFtpConfiguration(configuration);
        }
        return configuration;
    }

    public void storeFtpConfiguration(FtpConfiguration configuration) {

        FtpContext ftpContext = ((FtpBackupPlan) configuration.getBackupPlan()).getFtpContext();

        getHibernateTemplate().saveOrUpdate(configuration);

        ftpContext.setHost(configuration.getHost());
        ftpContext.setPassword(configuration.getPassword());
        ftpContext.setUserId(configuration.getUserId());
    }

    public LocalBackupPlan getLocalBackupPlan() {
        List plans = getHibernateTemplate().loadAll(LocalBackupPlan.class);
        LocalBackupPlan plan = (LocalBackupPlan) DaoUtils.requireOneOrZero(plans, "all backup plans");
        if (plan == null) {
            plan = (LocalBackupPlan) m_beanFactory.getBean("localBackupPlan");
            storeBackupPlan(plan);
        }
        return plan;
    }

    public void storeBackupPlan(BackupPlan plan) {
        getHibernateTemplate().saveOrUpdate(plan);
        resetTimer(plan);
    }

    public File[] performBackup(BackupPlan plan) {
        String backupDirectory = (plan instanceof FtpBackupPlan) ? m_ftpBackupDirectory : m_backupDirectory;
        return plan.perform(backupDirectory, m_binDirectory);
    }

    public void performExport(Writer writer) throws IOException {
        m_exportCsv.exportCsv(writer);

    }

    /**
     * start backup timers after app is initialized
     */
    public void onApplicationEvent(ApplicationEvent event) {
        // No need to register listener, all beans that implement listener
        // interface are
        // automatically registered
        if (event instanceof ApplicationInitializedEvent || event instanceof DSTChangeEvent) {
            resetTimer(getLocalBackupPlan());
            resetTimer(getFtpConfiguration().getBackupPlan());
        }
    }

    private void resetTimer(BackupPlan plan) {
        Timer timer = plan.getTimer();
        if (timer != null) {
            timer.cancel();
        }
        timer = new Timer(false); // daemon, dies with main thread
        plan.setTimer(timer);
        String backupDirectory = (plan instanceof FtpBackupPlan) ? m_ftpBackupDirectory : m_backupDirectory;
        plan.schedule(timer, backupDirectory, m_binDirectory);
    }

    public String[] getInitializationTasks() {
        List l = getHibernateTemplate().findByNamedQuery("taskNames");
        return (String[]) l.toArray(new String[l.size()]);
    }

    public void deleteInitializationTask(String task) {
        List l = getHibernateTemplate().findByNamedQueryAndNamedParam("taskByName", "task", task);
        getHibernateTemplate().deleteAll(l);
    }

    /**
     * Prepares a list of backup files that can be used to restore sipX configuration or sipX
     * voice mail.
     * 
     * Each list items is a map: backup type -> backup bean(file name). Each list items contains
     * files from a single directory. List is sorted by directory name in the backup order.
     * 
     */
    public List<Map<Type, BackupBean>> getBackups() {

        File backupDirectory = new File(getBackupDirectory());
        FileFilter dirFilter = DirectoryFileFilter.DIRECTORY;
        File [] backupFolders = backupDirectory.listFiles(dirFilter);

        if (backupFolders == null) {
            return Collections.emptyList();
        }
        List<Map<Type, BackupBean>> backupBeans = new ArrayList<Map<Type, BackupBean>>();
        for (File backupFolder : backupFolders) {
            File [] backupFiles = backupFolder.listFiles(BackupPlan.BACKUP_FILE_FILTER);
            addBackupBeans(backupBeans, backupFiles);
        }
        Collections.sort(backupBeans, new BackupBean.CompareFolders());
        return backupBeans;
    }
    /**
     * create and add BackupBeans
     * @param repositoryBean
     * @param backupFiles
     */
    private  void addBackupBeans(List<Map<Type, BackupBean>> repositoryBean, File [] backupFiles) {
        Map<Type, BackupBean> backups = new HashMap<Type, BackupBean>(2);
        for (File file : backupFiles) {
            BackupBean backupBean = new BackupBean(file);
            backups.put(backupBean.getType(), backupBean);

        }
        if (!backups.isEmpty()) {
            repositoryBean.add(backups);
        }
    }
    /**
     * The same as getBackups but the file paths are constructed in such a way
     * as to match with FTP download location
     */
    public List<Map<Type, BackupBean>> getFtpBackups() {
        File [] backupFolders = null;
        FtpContext ftpContext = ((FtpBackupPlan) getFtpConfiguration().getBackupPlan()).getFtpContext();
        ftpContext.openConnection();
        String [] directoryNames = ftpContext.list(".");
        backupFolders = new File [directoryNames.length];
        int i = 0;
        for (String directoryName : directoryNames) {
            backupFolders[i++] = new File(getFtpBackupDirectory() + File.separator + directoryName);
        }
        List<Map<Type, BackupBean>> backupBeans = new ArrayList<Map<Type, BackupBean>>();
        for (File backupFolder : backupFolders) {
            String [] names = ftpContext.list(backupFolder.getName());
            File[] backupFiles = new File[names.length];
            i = 0;
            for (String name : names) {
                backupFiles[i++] = new File(backupFolder.getAbsolutePath() + File.separator + name);
            }
            addBackupBeans(backupBeans, backupFiles);
        }
        ftpContext.closeConnection();
        Collections.sort(backupBeans, new BackupBean.CompareFolders());
        return backupBeans;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public boolean inUpgradePhase() {
        // HACK: need to find a better way of finding out if this is upgrade or normal (tapestry)
        // for now we are assuming that "tapestry" bean is not available during upgrade run
        return !m_beanFactory.containsBean("tapestry");
    }


    public String getFtpBackupDirectory() {
        return m_ftpBackupDirectory;
    }

    public void setFtpBackupDirectory(String ftpBackupDirectory) {
        m_ftpBackupDirectory = ftpBackupDirectory;
    }

    public void setSystemDate(Date date) {
        String errorMsg = "Error when changing date";
        ProcessBuilder pb = new ProcessBuilder(
                getLibExecDirectory() + File.separator + DATE_BINARY);

        pb.command().add(CHANGE_DATE_FORMAT.format(date));
        try {
            LOG.debug(pb.command());
            Process process = pb.start();
            BufferedReader scriptErrorReader = new BufferedReader(new InputStreamReader(process
                    .getErrorStream()));
            String errorLine = scriptErrorReader.readLine();
            while (errorLine != null) {
                LOG.warn("sipx-sudo-date: " + errorLine);
                errorLine = scriptErrorReader.readLine();
            }
            int code = process.waitFor();
            if (code != 0) {
                errorMsg = String.format("Error when changing date. Exit code: %d", code);
                LOG.error(errorMsg);
            }
        } catch (IOException e) {
            LOG.error(errorMsg, e);
        } catch (InterruptedException e) {
            LOG.error(errorMsg, e);
        }
    }

}
