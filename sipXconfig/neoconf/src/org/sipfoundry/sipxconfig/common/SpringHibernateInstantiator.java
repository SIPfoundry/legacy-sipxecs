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
import java.lang.reflect.Proxy;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

import org.apache.commons.collections.Transformer;
import org.apache.commons.collections.map.LazyMap;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.CallbackException;
import org.hibernate.EmptyInterceptor;
import org.hibernate.EntityMode;
import org.hibernate.SessionFactory;
import org.hibernate.metadata.ClassMetadata;
import org.hibernate.type.Type;
import org.sipfoundry.sipxconfig.common.event.HibernateEntityChangeProvider;
import org.sipfoundry.sipxconfig.systemaudit.ConfigChangeAction;
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
    private Map<String, EntityDecorator> m_decorators;
    private Collection<HibernateEntityChangeProvider> m_hbEntityProviders;

    private Set<HbEntity> m_inserts = new HashSet<HbEntity>();
    private Set<HbEntity> m_updates = new HashSet<HbEntity>();
    private Set<HbEntity> m_deletes = new HashSet<HbEntity>();

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

        BeanWithId bean = m_beanFactory.getBean(beanName, BeanWithId.class);
        bean.setId((Integer) id);

        EntityDecorator decorator = getDecorator(clazz);
        if (decorator != null) {
            decorator.decorateEntity(bean, id);
        }

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

    @Override
    public boolean onSave(Object entity, Serializable id, Object[] state, String[] propertyNames, Type[] types) {
        EntityDecorator decorator = getDecorator(entity);
        if (decorator != null) {
            decorator.onSave(entity, id);
        }
        m_inserts.add(new HbEntity(entity, id, null, null, propertyNames, types, state));
        return super.onSave(entity, id, state, propertyNames, types);
    }

    @Override
    public void onDelete(Object entity, Serializable id, Object[] state, String[] propertyNames, Type[] types) {
        EntityDecorator decorator = getDecorator(entity);
        if (decorator != null) {
            decorator.onDelete(entity, id);
        }
        m_deletes.add(new HbEntity(entity, id, null, null, propertyNames, types, state));
        super.onDelete(entity, id, state, propertyNames, types);
    }

    private EntityDecorator getDecorator(Object entity) {
        return getDecorator(entity.getClass());
    }

    private EntityDecorator getDecorator(Class clazz) {
        String decoratorName = clazz.getSimpleName().toLowerCase() + "Decorator";
        if (getEntityDecorators().containsKey(decoratorName)) {
            EntityDecorator decorator = getEntityDecorators().get(decoratorName);
            return decorator;
        }
        return null;
    }

    private Map<String, EntityDecorator> getEntityDecorators() {
        if (m_decorators == null) {
            m_decorators = m_beanFactory.getBeansOfType(EntityDecorator.class, false, false);
        }
        return m_decorators;
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
    public boolean onFlushDirty(Object obj, Serializable id, Object[] newValues, Object[] oldValues,
            String[] properties, Type[] types) throws CallbackException {
        m_updates.add(new HbEntity(obj, id, newValues, oldValues, properties, types, null));
        return super.onFlushDirty(obj, id, oldValues, newValues, properties, types);
    }

    @Override
    public void onCollectionUpdate(Object collection, Serializable key) throws CallbackException {
        for (HibernateEntityChangeProvider provider : getHbEntityChangeProviders()) {
            provider.onConfigChangeCollectionUpdate(collection, key);
        }
        super.onCollectionUpdate(collection, key);
    }

    public void postFlush(Iterator iterator) {
        HbEntity hbEntity = null;
        try {
            for (Iterator<HbEntity> it = m_inserts.iterator(); it.hasNext();) {
                hbEntity = it.next();
                for (HibernateEntityChangeProvider provider : getHbEntityChangeProviders()) {
                    provider.onConfigChangeAction(hbEntity.getEntity(), ConfigChangeAction.ADDED,
                        hbEntity.getProperties(), null, null);
                }
            }
            for (Iterator<HbEntity> it = m_updates.iterator(); it.hasNext();) {
                hbEntity = it.next();
                for (HibernateEntityChangeProvider provider : getHbEntityChangeProviders()) {
                    provider.onConfigChangeAction(hbEntity.getEntity(), ConfigChangeAction.MODIFIED,
                        hbEntity.getProperties(), hbEntity.getOldValues(), hbEntity.getNewValues());
                }
            }
            for (Iterator<HbEntity> it = m_deletes.iterator(); it.hasNext();) {
                hbEntity = it.next();
                for (HibernateEntityChangeProvider provider : getHbEntityChangeProviders()) {
                    provider.onConfigChangeAction(hbEntity.getEntity(), ConfigChangeAction.DELETED,
                        hbEntity.getProperties(), null, null);
                }
            }

        } finally {
            m_inserts.clear();
            m_updates.clear();
            m_deletes.clear();
        }
    }

    private Collection<HibernateEntityChangeProvider> getHbEntityChangeProviders() {
        if (m_hbEntityProviders == null) {
            m_hbEntityProviders = new ArrayList<HibernateEntityChangeProvider>();
            Map<String, HibernateEntityChangeProvider> beanMap = m_beanFactory
                    .getBeansOfType(HibernateEntityChangeProvider.class, false,
                            false);
            for (HibernateEntityChangeProvider provider : beanMap.values()) {
                if (provider instanceof Proxy) {
                    m_hbEntityProviders.add(provider);
                }
            }
        }
        return m_hbEntityProviders;
    }
}
