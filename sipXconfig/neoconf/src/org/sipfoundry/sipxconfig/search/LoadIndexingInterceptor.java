/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.search;

import java.io.Serializable;

import org.hibernate.SessionFactory;
import org.hibernate.event.EventListeners;
import org.hibernate.event.PostLoadEvent;
import org.hibernate.event.PostLoadEventListener;
import org.hibernate.impl.SessionFactoryImpl;
import org.hibernate.type.Type;
import org.sipfoundry.sipxconfig.common.SpringHibernateInstantiator;
import org.sipfoundry.sipxconfig.common.event.KeepsOriginalCopy;

/**
 * This is used to indexing on load
 *   ...AND completely unrelated...
 * support KeepOriginalCopy interface.
 */
public class LoadIndexingInterceptor extends SpringHibernateInstantiator implements PostLoadEventListener {
    private Indexer m_indexer;
    private BeanIndexHelper m_beanIndexHelper;

    public void setIndexer(Indexer indexer) {
        m_indexer = indexer;
    }

    public void setBeanIndexHelper(BeanIndexHelper beanIndexHelper) {
        m_beanIndexHelper = beanIndexHelper;
    }

    public boolean onLoad(Object entity, Serializable id, Object[] state, String[] propertyNames,
            Type[] types) {
        BeanIndexProperties beanIndexProperties = new BeanIndexProperties(entity, id, state, propertyNames, types);
        m_beanIndexHelper.setupIndexProperties(beanIndexProperties, true);
        m_indexer.indexBean(entity, id, beanIndexProperties.getState(), beanIndexProperties.getPropertyNames(),
                beanIndexProperties.getTypes(), true);
        return false;
    }

    @Override
    public void setSessionFactory(SessionFactory sessionFactory) {
        super.setSessionFactory(sessionFactory);
        if (sessionFactory instanceof SessionFactoryImpl) {
            SessionFactoryImpl impl = (SessionFactoryImpl) sessionFactory;
            EventListeners elisteners = impl.getEventListeners();
            addSelfAsPostLoadEventListener(elisteners);
        }
    }

    void addSelfAsPostLoadEventListener(EventListeners elisteners) {
        PostLoadEventListener[] listeners = elisteners.getPostLoadEventListeners();
        if (listeners == null || listeners.length == 0) {
            listeners = new PostLoadEventListener[1];
        } else {
            PostLoadEventListener[] copy = new PostLoadEventListener[listeners.length + 1];
            System.arraycopy(listeners, 0, copy, 0, listeners.length);
        }
        listeners[listeners.length - 1] = this;
        elisteners.setPostLoadEventListeners(listeners);
    }

    @Override
    public void onPostLoad(PostLoadEvent arg0) {
        Object entity = arg0.getEntity();
        if (entity instanceof KeepsOriginalCopy) {
            ((KeepsOriginalCopy<?>) entity).makeBackupAsOriginalCopy();
        }
    }
}
