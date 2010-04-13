/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.lang.reflect.Proxy;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.BeanInitializationException;
import org.springframework.beans.factory.ListableBeanFactory;

/**
 * Finds all the beans that implement AliasProvider interface and retrieves the aliases for that
 * beans
 */
public class AliasCollector implements AliasProvider, BeanFactoryAware {

    private Collection m_aliasProviders;

    private ListableBeanFactory m_beanFactory;

    private List m_aliasProviderBeanIds = Collections.EMPTY_LIST;

    private boolean m_includeProxies;

    public Collection getAliasMappings() {
        Collection aliasProviders = getAliasProviders();
        Collection aliasMappings = new ArrayList();
        for (Iterator i = aliasProviders.iterator(); i.hasNext();) {
            AliasProvider provider = (AliasProvider) i.next();
            aliasMappings.addAll(provider.getAliasMappings());
        }

        return aliasMappings;
    }

    /**
     * Lazily creates the collection of beans that implement AliasProvider interface
     *
     *
     * @return cached or newly created listener collection
     */
    protected Collection getAliasProviders() {
        if (m_aliasProviders == null) {
            if (m_beanFactory == null) {
                throw new BeanInitializationException(getClass().getName() + " not initialized");
            }
            Map beanMap = m_beanFactory.getBeansOfType(AliasProvider.class, false, true);
            m_aliasProviders = new ArrayList(beanMap.size());

            if (m_includeProxies) {
                // collect all proxies
                for (Iterator i = beanMap.values().iterator(); i.hasNext();) {
                    AliasProvider provider = (AliasProvider) i.next();
                    // only include beans created through Factories - need hibernate support
                    if (provider instanceof Proxy) {
                        m_aliasProviders.add(provider);
                    }
                }
            }
            // collect additional beans
            for (Iterator i = m_aliasProviderBeanIds.iterator(); i.hasNext();) {
                String beanId = (String) i.next();
                Object bean = m_beanFactory.getBean(beanId, AliasProvider.class);
                m_aliasProviders.add(bean);
            }
        }
        return m_aliasProviders;
    }

    public void setIncludeProxies(boolean includeProxies) {
        m_includeProxies = includeProxies;
    }

    public void setAliasProviderBeanIds(List aliasProviderBeanIds) {
        m_aliasProviderBeanIds = aliasProviderBeanIds;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
        m_aliasProviders = null;
    }
}
