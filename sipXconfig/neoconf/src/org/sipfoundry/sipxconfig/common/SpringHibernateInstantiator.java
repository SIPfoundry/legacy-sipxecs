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

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.apache.commons.collections.Transformer;
import org.apache.commons.collections.map.LazyMap;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.EmptyInterceptor;
import org.hibernate.EntityMode;
import org.hibernate.SessionFactory;
import org.hibernate.metadata.ClassMetadata;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

/**
 * Enables Spring to create the hibernate object. Use to allow Spring to manage object
 * dependencies with hibernate.
 *
 * Note: it inherits from IndexingInterceptor: only one interceptor can be registered with
 * hibernate session.
 */
public class SpringHibernateInstantiator extends EmptyInterceptor implements BeanFactoryAware {
    private static final Log LOG = LogFactory.getLog(SpringHibernateInstantiator.class);
    private ListableBeanFactory m_beanFactory;
    private SessionFactory m_sessionFactory;
    private Map m_beanNamesCache;
    private SipxReplicationContext m_sipxReplicationContext;

    /**
     * This implementation only supports BeanWithId objects with integer ids
     */
    public Object instantiate(String entityName, EntityMode entityMode, Serializable id) {
        ClassMetadata classMetadata = m_sessionFactory.getClassMetadata(entityName);
        Class clazz = classMetadata.getMappedClass(entityMode);
        return instantiate(clazz, id);
    }

    Object instantiate(Class clazz, Serializable id) {
        String beanName = (String) m_beanNamesCache.get(clazz);
        if (beanName == null) {
            return null;
        }

        BeanWithId bean = (BeanWithId) m_beanFactory.getBean(beanName, BeanWithId.class);
        bean.setId((Integer) id);
        return bean;
    }

    private static class ClassToBeanName implements Transformer {
        private ListableBeanFactory m_beanFactory;

        ClassToBeanName(ListableBeanFactory beanFactory) {
            m_beanFactory = beanFactory;
        }

        public Object transform(Object input) {
            Class clazz = (Class) input;
            String[] beanDefinitionNames = m_beanFactory.getBeanNamesForType(clazz);
            LOG.debug(beanDefinitionNames.length + " beans registered for class: " + clazz.getName());
            for (int i = 0; i < beanDefinitionNames.length; i++) {
                Object bean = m_beanFactory.getBean(beanDefinitionNames[i]);

                if (clazz == bean.getClass()) {
                    // only return the bean name if class matches exactly - no
                    // subclasses
                    return beanDefinitionNames[i];
                }
            }
            return null;
        }
    }

    /**
     * This can only be used withy listeable bean factory
     */
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
        Transformer transformer = new ClassToBeanName(m_beanFactory);
        m_beanNamesCache = LazyMap.decorate(new HashMap(), transformer);
    }

    public BeanFactory getBeanFactory() {
        return m_beanFactory;
    }

    public void setSessionFactory(SessionFactory sessionFactory) {
        m_sessionFactory = sessionFactory;
    }

    @Override
    public void postFlush(Iterator entities) {
        // we need a synchronized collection here otherwise we'll get a ConcurrentModification.
        List<Replicable> list = new ArrayList<Replicable>();
        while (entities.hasNext()) {
            Object o = entities.next();
            if (o instanceof Replicable) {
                list.add((Replicable) o);
            }
        }
        for (Replicable repl : Collections.synchronizedList(list)) {
            m_sipxReplicationContext.replicateWork(repl);
        }
    }

    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }
}
