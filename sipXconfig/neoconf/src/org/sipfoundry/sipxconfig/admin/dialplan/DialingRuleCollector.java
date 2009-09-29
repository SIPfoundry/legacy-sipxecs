/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

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

public class DialingRuleCollector implements DialingRuleProvider, BeanFactoryAware {
    private Collection<DialingRuleProvider> m_dialingRuleProviders;
    private ListableBeanFactory m_beanFactory;

    public List<DialingRule> getDialingRules() {
        List<DialingRule> dialingRules = new ArrayList<DialingRule>();
        Collection<DialingRuleProvider> dialingRuleProviders = getDialingRuleProviders();
        for (DialingRuleProvider provider : dialingRuleProviders) {
            for (DialingRule rule : provider.getDialingRules()) {
                rule.appendToGenerationRules(dialingRules);
            }
        }
        return dialingRules;
    }

    /**
     * Lazily creates the collection of beans that implement the DialingRuleProvider interface
     *
     * @return cached or newly created listener collection
     */
    protected Collection<DialingRuleProvider> getDialingRuleProviders() {
        if (m_dialingRuleProviders == null) {
            if (m_beanFactory == null) {
                throw new BeanInitializationException(getClass().getName() + " not initialized");
            }
            Map beanMap = m_beanFactory.getBeansOfType(DialingRuleProvider.class, false, true);
            m_dialingRuleProviders = new ArrayList(beanMap.size());
            // collect all proxies
            for (Iterator i = beanMap.values().iterator(); i.hasNext();) {
                DialingRuleProvider provider = (DialingRuleProvider) i.next();
                // only include beans created through Factories - need hibernate support
                if (provider instanceof Proxy) {
                    m_dialingRuleProviders.add(provider);
                }
            }
        }
        return m_dialingRuleProviders;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
        m_dialingRuleProviders = null;
    }
}
