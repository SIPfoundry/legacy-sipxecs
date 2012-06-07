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

import static java.lang.String.format;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.apache.commons.io.FileUtils;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class BackupManagerImpl extends HibernateDaoSupport implements BackupManager,
        BeanFactoryAware {
    private FeatureManager m_featureManager;
    private Collection<ArchiveProvider> m_providers;
    private ListableBeanFactory m_beanFactory;
    private BeanWithSettingsDao<BackupSettings> m_settingsDao;
    private LocationsManager m_locationsManager;
    private ConfigManager m_configManager;
    private String m_backupScript;
    private File m_restoreStagingDir;

    @Override
    public void saveSettings(BackupSettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public BackupSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public BackupPlan findOrCreateBackupPlan(BackupType type) {
        for (BackupPlan plan : getBackupPlans()) {
            if (plan.getType() == type) {
                return plan;
            }
        }
        BackupPlan plan = new BackupPlan();
        plan.setType(type);
        return plan;
    }

    @Override
    public void saveBackupPlan(BackupPlan plan) {
        getHibernateTemplate().saveOrUpdate(plan);
    }

    @Override
    public Collection<ArchiveDefinition> getArchiveDefinitions(Location location) {
        Set<ArchiveDefinition> defs = new HashSet<ArchiveDefinition>();
        for (ArchiveProvider provider : getArchiveProviders()) {
            Collection<ArchiveDefinition> locationDefs = provider.getArchiveDefinitions(this, location);
            if (locationDefs != null) {
                defs.addAll(locationDefs);
            }
        }

        return defs;
    }

    @Override
    public FeatureManager getFeatureManager() {
        return m_featureManager;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    public Collection<ArchiveProvider> getArchiveProviders() {
        if (m_providers == null) {
            Map<String, ArchiveProvider> beanMap = m_beanFactory.getBeansOfType(ArchiveProvider.class, false, false);
            m_providers = new ArrayList<ArchiveProvider>(beanMap.values());
        }

        return m_providers;
    }

    public void restore(ArchiveDefinition def, Location location, File f) {
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public void setSettingsDao(BeanWithSettingsDao<BackupSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Collection<BackupPlan> getBackupPlans() {
        return getHibernateTemplate().loadAll(BackupPlan.class);
    }

    @Override
    public Collection<String> getArchiveDefinitionIds() {
        Set<String> ids = new HashSet<String>();
        for (Location location : m_locationsManager.getLocationsList()) {
            for (ArchiveProvider provider : getArchiveProviders()) {
                Collection<ArchiveDefinition> defs = provider.getArchiveDefinitions(this, location);
                if (defs != null) {
                    for (ArchiveDefinition def : defs) {
                        ids.add(def.getId());
                    }
                }
            }
        }

        return ids;
    }

    @Override
    public String getBackupLink(BackupPlan plan) {
        BackupCommandRunner runner = new BackupCommandRunner(getPlanFile(plan), getBackupScript());
        return runner.getBackupLink();
    }

    public File getPlanFile(BackupPlan plan) {
        String fname = format("1/archive-%s.yaml", plan.getType());
        return new File(m_configManager.getGlobalDataDirectory(), fname);
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public String getBackupScript() {
        return m_backupScript;
    }

    public void setBackupScript(String backupScript) {
        m_backupScript = backupScript;
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public void setRestoreStagingDirectoryPath(String dir) {
        m_restoreStagingDir = new File(dir);
    }

    @Override
    public File getRestoreStagingDirectory() {
        return m_restoreStagingDir;
    }

    @Override
    public File getCleanRestoreStagingDirectory() {
        File dir = getRestoreStagingDirectory();
        try {
            FileUtils.deleteDirectory(dir);
        } catch (IOException e) {
            throw new RuntimeException("Failure to get fresh restore directory", e);
        }
        dir.mkdirs();
        return dir;
    }

}
