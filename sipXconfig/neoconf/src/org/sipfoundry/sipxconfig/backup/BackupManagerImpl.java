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

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.sipxconfig.commserver.Location;
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
    public void storeBackupPlan(BackupPlan plan) {
        getHibernateTemplate().saveOrUpdate(plan);
    }

    @Override
    public Collection<ArchiveDefinition> getArchiveDefinitions(BackupPlan plan, Location location) {
        return getArchiveDefinitions(plan.getDefinitionIds(), location);
    }

    @Override
    public Collection<ArchiveDefinition> getArchiveDefinitions(Collection<String> definitionIds, Location location) {
        Set<ArchiveDefinition> defs = new HashSet<ArchiveDefinition>();
        for (ArchiveProvider provider : getArchiveProviders()) {
            Collection<ArchiveDefinition> locationDefs = provider.getArchiveDefinitions(this, location);
            if (locationDefs != null) {
                for (ArchiveDefinition def : locationDefs) {
                    if (definitionIds.contains(def.getId())) {
                        defs.add(def);
                    }
                }
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
}
