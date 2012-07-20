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
package org.sipfoundry.sipxconfig.snmp;


import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.ServiceStatus;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class SnmpManagerImpl implements BeanFactoryAware, SnmpManager, FeatureProvider, ProcessProvider {
    private ListableBeanFactory m_beanFactory;
    private FeatureManager m_featureManager;
    private Collection<ProcessProvider> m_processProviders;
    private BeanWithSettingsDao<SnmpSettings> m_settingsDao;

    @Override
    public List<ProcessDefinition> getProcessDefinitions(Location location) {
        List<ProcessDefinition> defs = new ArrayList<ProcessDefinition>();
        for (ProcessProvider provider : getProcessProviders()) {
            Collection<ProcessDefinition> pdefs = provider.getProcessDefinitions(this, location);
            if (pdefs != null) {
                defs.addAll(pdefs);
            }
        }
        return defs;
    }

    Collection<ProcessProvider> getProcessProviders() {
        if (m_processProviders == null) {
            Map<String, ProcessProvider> beansOfType = m_beanFactory.getBeansOfType(ProcessProvider.class);
            m_processProviders = beansOfType.values();
        }
        return m_processProviders;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    public FeatureManager getFeatureManager() {
        return m_featureManager;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    @Override
    public List<ServiceStatus> getServicesStatuses(Location location) {
        ProcessSnmpReader reader = new ProcessSnmpReader();
        try {
            List<ServiceStatus> statuses = reader.read(location.getAddress());
            return statuses;
        } catch (IOException e) {
            throw new UserException("Could not get SNMP data", e);
        }
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return Collections.singleton(FEATURE);
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return null;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE);
        return (enabled ? Collections.singleton(ProcessDefinition.sysvDefault("snmpd")) : null);
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
    }

    @Override
    public SnmpSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(SnmpSettings settings) {
        m_settingsDao.upsert(settings);
    }

    public void setSettingsDao(BeanWithSettingsDao<SnmpSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }
}
