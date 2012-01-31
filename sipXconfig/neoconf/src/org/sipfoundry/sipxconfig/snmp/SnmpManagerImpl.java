/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.snmp;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class SnmpManagerImpl implements BeanFactoryAware, SnmpManager {
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
}
