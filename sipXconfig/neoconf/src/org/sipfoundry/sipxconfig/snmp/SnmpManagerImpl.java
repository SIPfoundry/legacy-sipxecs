/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class SnmpManagerImpl implements BeanFactoryAware, SnmpManager, FeatureProvider, ProcessProvider {
    private ListableBeanFactory m_beanFactory;
    private FeatureManager m_featureManager;
    private Collection<ProcessProvider> m_processProviders;

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
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return Collections.singleton(FEATURE);
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return null;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE);
        return (enabled ? Collections.singleton(new ProcessDefinition("snmpd")) : null);
    }

    @Override
    public void getBundleFeatures(Bundle b) {
        if (b.isRouter()) {
            b.addFeature(FEATURE);
        }
    }
}
