/**
 *
 *
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

import static org.springframework.dao.support.DataAccessUtils.singleResult;

import java.io.File;
import java.util.List;

import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.DSTChangeEvent;
import org.sipfoundry.sipxconfig.ftp.FtpExternalServerConfig;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public abstract class BackupManagerImpl extends HibernateDaoSupport implements ApplicationListener, BackupManager {
    private String m_binDirectory;
    private String m_libExecDirectory;

    @Override
    public abstract FtpBackupPlan createFtpBackupPlan();

    @Override
    public abstract LocalBackupPlan createLocalBackupPlan();

    /* (non-Javadoc)
     * @see org.sipfoundry.sipxconfig.backup.BackupManager#getBackupPlan(java.lang.String)
     */
    @Override
    public BackupPlan getBackupPlan(String type) {
        boolean isFtp = FtpBackupPlan.TYPE.equals(type);
        Class klass = isFtp ? FtpBackupPlan.class : LocalBackupPlan.class;
        List plans = getHibernateTemplate().loadAll(klass);
        BackupPlan plan = (BackupPlan) DataAccessUtils.singleResult(plans);
        if (plan == null) {
            plan = isFtp ? createFtpBackupPlan() : createLocalBackupPlan();
            if (isFtp) {
                initFtpConfig(plan);
            }
            getHibernateTemplate().save(plan);
        }
        return plan;
    }

    private void initFtpConfig(BackupPlan plan) {
        FtpBackupPlan ftpBackupPlan = (FtpBackupPlan) plan;
        if (ftpBackupPlan.getFtpConfiguration() != null) {
            return;
        }
        List ftpConfigs = getHibernateTemplate().loadAll(FtpExternalServerConfig.class);
        FtpExternalServerConfig ftpConfig = (FtpExternalServerConfig) singleResult(ftpConfigs);
        if (ftpConfig == null) {
            ftpConfig = new FtpExternalServerConfig();
        }
        ftpBackupPlan.setFtpConfiguration(ftpConfig);
    }

    /* (non-Javadoc)
     * @see org.sipfoundry.sipxconfig.backup.BackupManager#storeBackupPlan(org.sipfoundry.sipxconfig.backup.BackupPlan)
     */
    @Override
    public void storeBackupPlan(BackupPlan plan) {
        getHibernateTemplate().saveOrUpdate(plan);
        plan.resetTimer(m_binDirectory);
    }

    /* (non-Javadoc)
     * @see org.sipfoundry.sipxconfig.backup.BackupManager#performBackup(org.sipfoundry.sipxconfig.backup.BackupPlan)
     */
    @Override
    public File[] performBackup(BackupPlan plan) {
        return plan.perform(m_binDirectory);
    }

    /**
     * start backup timers after app is initialized
     */
    public void onApplicationEvent(ApplicationEvent event) {
        // No need to register listener, all beans that implement listener
        // interface are
        // automatically registered
        if (event instanceof ApplicationInitializedEvent || event instanceof DSTChangeEvent) {
            List<BackupPlan> plans = getHibernateTemplate().loadAll(BackupPlan.class);
            for (BackupPlan plan : plans) {
                plan.resetTimer(m_binDirectory);
            }
        }
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
}
