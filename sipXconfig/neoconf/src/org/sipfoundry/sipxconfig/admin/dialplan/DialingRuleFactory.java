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

import java.util.ArrayList;
import java.util.Collection;

import org.apache.commons.lang.StringUtils;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;

public class DialingRuleFactory implements BeanFactoryAware {
    private Collection<String> m_beanIds;
    private Collection<String> m_filteredBeanIds;

    private BeanFactory m_beanFactory;

    /**
     * Constructs dialing rule from prototypes defined in Spring configuration file.
     *
     * Throws illegal argument exception if invalid or unregistered type is passed.
     *
     * @param type dialing rule type
     * @return newly created object
     */
    public DialingRule create(String beanId) {
        DialingRule rule = (DialingRule) m_beanFactory.getBean(beanId, DialingRule.class);
        // reset new rule - we do not want to suggest invalid values for name, description etc.
        rule.setEnabled(false);
        rule.setDescription(StringUtils.EMPTY);
        rule.setName(StringUtils.EMPTY);
        return rule;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public void setBeanIds(Collection<String> beanIds) {
        m_beanIds = beanIds;
        m_filteredBeanIds = null;
    }

    /**
     * @return beanIds that are defined in bean factory and can be used as prototypes for rules
     */
    public Collection<String> getBeanIds() {
        if (m_filteredBeanIds == null) {
            m_filteredBeanIds = filterBeanIds(m_beanIds);
        }
        return m_filteredBeanIds;
    }

    private Collection<String> filterBeanIds(Collection<String> beanIds) {
        ArrayList<String> filteredBeanIds = new ArrayList<String>(beanIds.size());
        for (String beanId : beanIds) {
            if (m_beanFactory.containsBean(beanId)) {
                filteredBeanIds.add(beanId);
            }
        }
        return filteredBeanIds;
    }
}
