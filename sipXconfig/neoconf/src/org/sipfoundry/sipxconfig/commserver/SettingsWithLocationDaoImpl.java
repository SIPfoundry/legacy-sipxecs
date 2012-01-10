/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
        getHibernateTemplate().saveOrUpdate(bean);
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
