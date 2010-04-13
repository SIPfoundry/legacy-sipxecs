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

import java.util.Collections;
import java.util.Formatter;
import java.util.List;

import org.hibernate.cfg.Configuration;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.BeanInitializationException;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.orm.hibernate3.LocalSessionFactoryBean;

public class DynamicSessionFactoryBean extends LocalSessionFactoryBean implements
        BeanFactoryAware {
    public static final String HEADER = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            + "<!DOCTYPE hibernate-mapping PUBLIC \"-//Hibernate/Hibernate Mapping DTD 3.0//EN\" "
            + "   \"http://hibernate.sourceforge.net/hibernate-mapping-3.0.dtd\">";
    public static final String MAPPING_PATTERN = "<hibernate-mapping default-lazy=\"false\">"
            + "<subclass name=\"%s\" extends=\"%s\" discriminator-value=\"%s\"/>"
            + "</hibernate-mapping>";

    private ListableBeanFactory m_beanFactory;

    /**
     * Collection of bean IDs that representing superclasses of the hierarchy. All beans of the
     * same type as bean ID will be automatically mapped to the same hibernate table.
     */
    private List<String> m_baseClassBeanIds = Collections.EMPTY_LIST;

    protected void postProcessConfiguration(Configuration config) {
        for (String baseClassBeanID : m_baseClassBeanIds) {
            bindSubclasses(config, baseClassBeanID);
        }
    }

    /**
     * Finds all subclasesses of baseClass in the bean factory and binds them to the same table as
     * base class using bean id as a discriminator value.
     *
     * @param config hibernate config that will be modified
     * @param baseClass base class - needs to be already mapped statically
     */
    protected void bindSubclasses(Configuration config, Class baseClass) {
        String[] beanDefinitionNames = m_beanFactory.getBeanNamesForType(baseClass);
        for (String beanId : beanDefinitionNames) {
            Class subClass = m_beanFactory.getType(beanId);
            if (subClass == baseClass) {
                continue; // skip baseclass which is already mapped
            }
            String mapping = xmlMapping(baseClass, subClass, beanId);
            config.addXML(mapping);
        }
    }

    /**
     * Finds all subclasesses of baseClass in the bean factory and binds them to the same table as
     * base class using bean id as a discriminator value.
     *
     * @param config hibernate config that will be modified
     * @param baseClassBeanId - bean representing the base class - needs to be already mapped
     *        statically
     */
    protected void bindSubclasses(Configuration config, String baseClassBeanId) {
        Class baseClass = m_beanFactory.getType(baseClassBeanId);
        bindSubclasses(config, baseClass);
    }

    /**
     * Create XML that contains a single subclass mapping
     *
     * @param baseClass already mapped hibernate entity class
     * @param subClass new entity to be mapped
     * @param discriminator value of disciminator for this subclass
     * @return xml string that can be parsed by hibernate to add new mapping
     */
    String xmlMapping(Class baseClass, Class subClass, String discriminator) {
        StringBuilder mapping = new StringBuilder(HEADER);
        Formatter formatter = new Formatter(mapping);
        formatter.format(MAPPING_PATTERN, subClass.getName(), baseClass.getName(), discriminator);
        return mapping.toString();
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        if (!(beanFactory instanceof ListableBeanFactory)) {
            throw new BeanInitializationException(getClass()
                    + " only works with ListableBeanFactory");
        }
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    public void setBaseClassBeanIds(List<String> baseClassBeanIds) {
        m_baseClassBeanIds = baseClassBeanIds;
    }
}
