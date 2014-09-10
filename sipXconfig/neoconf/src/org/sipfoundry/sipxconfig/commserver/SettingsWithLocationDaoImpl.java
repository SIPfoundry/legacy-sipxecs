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
package org.sipfoundry.sipxconfig.commserver;

import java.util.List;

import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class SettingsWithLocationDaoImpl<T extends SettingsWithLocation> extends SipxHibernateDaoSupport<T> implements
        SettingsWithLocationDao<T>, BeanFactoryAware {
    private static final String FIND_QUERY = "settingByLocation";
    private Class<T> m_class;
    private ListableBeanFactory m_beanFactory;

    public SettingsWithLocationDaoImpl(Class<T> type) {
        m_class = type;
    }

    @Override
    public List<T> findAll() {
        return (List<T>) getHibernateTemplate().loadAll(m_class);
    }

    @Override
    public T findOrCreate(Location location) {
        List<T> all = findAll(location);
        T settings = DaoUtils.requireOneOrZero(all, FIND_QUERY);
        return settings == null ? m_beanFactory.getBean(m_class) : settings;
    }

    @Override
    public void upsert(T bean) {
        if (bean.isNew()) {
            getHibernateTemplate().save(bean);
        } else {
            getHibernateTemplate().merge(bean);
        }
    }

    @Override
    public List<T> findAll(Location location) {
        List<T> results = getHibernateTemplate().findByNamedQuery(FIND_QUERY, location.getId());
        return results;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }
}
