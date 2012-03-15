/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.site;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.BeanInitializationException;
import org.springframework.beans.factory.ListableBeanFactory;

/**
 * Find and manages custom pages loaded in plugin jars
 */
public class PluginHookManager implements BeanFactoryAware {
    private ListableBeanFactory m_beanFactory;
    private List<PluginHook> m_hooks;

    @Override
    public void setBeanFactory(BeanFactory bf) {
        m_beanFactory = (ListableBeanFactory) bf;
        m_hooks = null;
    }

    public List<PluginHook> getHooks() {
        if (m_hooks == null) {
            if (m_beanFactory == null) {
                throw new BeanInitializationException(getClass().getName() + " not initialized");
            }
            Map<String, PluginHook> beanMap = m_beanFactory.getBeansOfType(PluginHook.class, false,
                    true);
            m_hooks = new ArrayList<PluginHook>(beanMap.values());
        }

        return m_hooks;
    }
}
