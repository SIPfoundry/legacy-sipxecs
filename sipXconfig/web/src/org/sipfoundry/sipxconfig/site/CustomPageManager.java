/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.BeanInitializationException;
import org.springframework.beans.factory.ListableBeanFactory;

/**
 * Find and manages custom pages loaded in plugin jars
 */
public class CustomPageManager implements BeanFactoryAware {

    private ListableBeanFactory m_beanFactory;
    private Collection<NavigationProvider> m_navProviders;

    @Override
    public void setBeanFactory(BeanFactory bf) {
        m_beanFactory = (ListableBeanFactory) bf;
        m_navProviders = null;
    }

    public Collection<String> getAdminMenuPageIds() {
        List<String> menus = new ArrayList<String>(getNavigationProviders().size());
        for (NavigationProvider n : getNavigationProviders()) {
            String pageName = n.getAdminMenuPageId();
            if (pageName != null) {
                menus.add(pageName);
            }
        }

        return menus;
    }

    public Collection<NavigationProvider> getNavigationProviders() {
        if (m_navProviders == null) {
            if (m_beanFactory == null) {
                throw new BeanInitializationException(getClass().getName() + " not initialized");
            }
            Map<String, NavigationProvider> beanMap = m_beanFactory.getBeansOfType(NavigationProvider.class, false,
                    true);
            m_navProviders = new ArrayList<NavigationProvider>(beanMap.values());
        }

        return m_navProviders;
    }
}
