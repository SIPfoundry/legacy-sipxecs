/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.config.BeanFactoryPostProcessor;
import org.springframework.beans.factory.config.ConfigurableListableBeanFactory;

public class SpringBeanPreInitializer implements BeanFactoryPostProcessor, BeanFactoryAware {
    private BeanFactory m_beanFactory;
    private String m_beanName;

    public void setBeanFactory(BeanFactory beanFactory)          {
        m_beanFactory = beanFactory;
    }

    public void setBeanName(String beanName) {
        m_beanName = beanName;
    }

    public void postProcessBeanFactory(ConfigurableListableBeanFactory beanFactory) {
        // initialize the bean by getting it by prefetching it from spring context
        m_beanFactory.getBean(m_beanName);
    }
}
