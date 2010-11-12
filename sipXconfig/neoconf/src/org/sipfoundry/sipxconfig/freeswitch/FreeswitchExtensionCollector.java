/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.freeswitch;

import java.lang.reflect.Proxy;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.BeanInitializationException;
import org.springframework.beans.factory.ListableBeanFactory;

public class FreeswitchExtensionCollector implements BeanFactoryAware {

    private ListableBeanFactory m_beanFactory;
    private Collection<FreeswitchExtensionProvider> m_freeswitchExtensionProviders;

    public List<FreeswitchExtension> getExtensions() {
        List<FreeswitchExtension> freeswitchExtensions = new ArrayList<FreeswitchExtension>();
        Collection<FreeswitchExtensionProvider> providers = getFreeswitchExtensionProviders();
        for (FreeswitchExtensionProvider provider : providers) {
            if (provider.isEnabled()) {
                freeswitchExtensions.addAll(provider.getFreeswitchExtensions());
            }
        }
        return freeswitchExtensions;
    }

    protected Collection<FreeswitchExtensionProvider> getFreeswitchExtensionProviders() {
        if (m_freeswitchExtensionProviders == null) {
            if (m_beanFactory == null) {
                throw new BeanInitializationException(getClass().getName() + " not initialized");
            }
            Map beanMap = m_beanFactory.getBeansOfType(FreeswitchExtensionProvider.class, false, true);
            m_freeswitchExtensionProviders = new ArrayList(beanMap.size());
            // collect all proxies
            for (Iterator i = beanMap.values().iterator(); i.hasNext();) {
                FreeswitchExtensionProvider provider = (FreeswitchExtensionProvider) i.next();
                // only include beans created through Factories - need hibernate support
                if (provider instanceof Proxy) {
                    m_freeswitchExtensionProviders.add(provider);
                }
            }
        }
        return m_freeswitchExtensionProviders;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
        m_freeswitchExtensionProviders = null;
    }

}
