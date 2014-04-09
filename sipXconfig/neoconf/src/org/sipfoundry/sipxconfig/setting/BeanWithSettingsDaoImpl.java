/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
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
        if (object.isNew()) {
            getHibernateTemplate().save(object);
        } else {
            getHibernateTemplate().merge(object);
        }
        getHibernateTemplate().flush();
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }
}
