/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.tunnel;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Map;

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.BeanInitializationException;
import org.springframework.beans.factory.ListableBeanFactory;

/**
 * Aggregate all tunnels for all locations and services and write them to stunnel config files
 * that map ports in and out.  This is useful for allowing services on one machine to securely communicate
 * with services on another machine without allowing unauthorized connections for services that either
 * don't have authentication mechanisms or are to cumbersome to configure such as the mongo database service.
 */
public class TunnelManagerImpl implements TunnelManager, BeanFactoryAware {
    private ListableBeanFactory m_beanFactory;
    private volatile Collection<TunnelProvider> m_providers;

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    /**
     * List of registered providers. Lazily get them from spring.
     */
    @Override
    public Collection<TunnelProvider> getTunnelProviders() {
        if (m_providers == null) {
            if (m_beanFactory == null) {
                throw new BeanInitializationException(getClass().getName() + " not initialized");
            }
            Map<String, TunnelProvider> beanMap = m_beanFactory.getBeansOfType(TunnelProvider.class, false,
                    true);
            m_providers = new ArrayList<TunnelProvider>(beanMap.values());
        }

        return m_providers;
    }

    public void setProviders(Collection<TunnelProvider> providers) {
        m_providers = providers;
    }
}
