/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.List;

import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class BeanWithSettingsDaoImpl<T extends BeanWithSettings> extends SipxHibernateDaoSupport<BeanWithSettings>
        implements BeanWithSettingsDao<T>, BeanFactoryAware {
    private Class<T> m_class;
    private ListableBeanFactory m_beanFactory;

    public BeanWithSettingsDaoImpl(String className) throws ClassNotFoundException {
        m_class = (Class<T>) Class.forName(className);
    }

    public BeanWithSettingsDaoImpl(Class<T> type) {
        m_class = type;
    }

    @Override
    public T findOrCreateOne() {
        List<T> all =  findAll();
        return all.isEmpty() ? m_beanFactory.getBean(m_class) : all.get(0);
    }

    @Override
    public List<T> findAll() {
        List<T> beans = (List<T>) getHibernateTemplate().loadAll(m_class);
        return beans;
    }

    @Override
    public void upsert(T object) {
        getHibernateTemplate().saveOrUpdate(object);
        getHibernateTemplate().flush();
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }
}
