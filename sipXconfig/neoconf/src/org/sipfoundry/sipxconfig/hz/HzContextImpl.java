/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.hz;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Map;

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.beans.factory.annotation.Required;

import com.hazelcast.config.Config;
import com.hazelcast.config.XmlConfigBuilder;
import com.hazelcast.core.Hazelcast;
import com.hazelcast.core.HazelcastInstance;

public class HzContextImpl implements HzContext, BeanFactoryAware {
    private static final String INSTANCE_NAME = "config-instance";
    private Collection<HzProvider> m_providers;
    private ListableBeanFactory m_beanFactory;

    @Override
    @Required
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    @Override
    public HazelcastInstance buildHzInstance(File f) throws FileNotFoundException {
        HazelcastInstance oldInstance = Hazelcast.getHazelcastInstanceByName(INSTANCE_NAME);
        if (oldInstance != null) {
            return oldInstance;
        }
        Config cfg = new XmlConfigBuilder(new FileInputStream(f)).build();
        cfg.setInstanceName(INSTANCE_NAME);
        HazelcastInstance instance = Hazelcast.newHazelcastInstance(cfg);
        for (HzProvider provider : getHzProviders()) {
            provider.configuretHzInstance(instance);
        }
        return instance;
    }

    private Collection<HzProvider> getHzProviders() {
        if (m_providers == null) {
            Map<String, HzProvider> beanMap = m_beanFactory.getBeansOfType(HzProvider.class, false, false);
            m_providers = new ArrayList<HzProvider>(beanMap.values());
        }

        return m_providers;
    }

}
