/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.commserver;

import java.util.List;

import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;

public class BeanWithLocationDaoImpl<T extends BeanWithLocation> extends SipxHibernateDaoSupport<T> implements
        BeanWithLocationDao<T> {

    private Class m_class;

    public BeanWithLocationDaoImpl(Class<T> type) {
        m_class = type;
    }

    @Override
    public List<T> findAll() {
        return (List<T>) getHibernateTemplate().loadAll(m_class);
    }

    @Override
    public T findOne(Location location) {
        throw new RuntimeException("TODO1");
    }

    @Override
    public void saveOrUpdate(T bean) {
        getHibernateTemplate().saveOrUpdate(bean);
    }

    @Override
    public List<T> findAll(Location location) {
        throw new RuntimeException("TODO2");
    }
}
