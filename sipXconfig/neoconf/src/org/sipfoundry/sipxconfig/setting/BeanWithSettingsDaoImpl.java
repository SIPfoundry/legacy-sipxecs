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

public class BeanWithSettingsDaoImpl<T> extends SipxHibernateDaoSupport<BeanWithSettings> implements
        BeanWithSettingsDao<T> {

    private Class m_class;

    public BeanWithSettingsDaoImpl(String className) throws ClassNotFoundException {
        m_class = Class.forName(className);
    }

    public BeanWithSettingsDaoImpl(Class<T> type) {
        m_class = type;
    }

    @Override
    public T findOne() {
        List<T> all =  findAll();
        return all.isEmpty() ? null : all.get(0);
    }

    @Override
    public List<T> findAll() {
        return (List<T>) getHibernateTemplate().loadAll(m_class);
    }

    @Override
    public void upsert(T object) {
        getHibernateTemplate().saveOrUpdate(object);
    }
}
