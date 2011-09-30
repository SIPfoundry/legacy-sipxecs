/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
