/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common.event;

import java.util.Collection;
import java.util.Iterator;
import java.util.Map;

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.BeanInitializationException;
import org.springframework.beans.factory.ListableBeanFactory;

/**
 * Publisher for Dao events
 *
 * Implementation: we could probably use Spring application context event publishing facility but
 * it would require that our event handling is aware of ApplicationEvent class.
 */
public class DaoEventPublisherImpl  implements DaoEventPublisher, BeanFactoryAware {
    private Collection m_listeners;

    private ListableBeanFactory m_beanFactory;

    /**
     * Lazily creates the collection of beans that implement DaoEventListener interface
     *
     * @return cached or newly created listener collection
     */
    private Collection getListeners() {
        if (m_listeners == null) {
            if (m_beanFactory == null) {
                throw new BeanInitializationException(getClass().getName() + " not initialized");
            }
            Map beanMap = m_beanFactory.getBeansOfType(DaoEventListener.class, true, true);
            m_listeners = beanMap.values();
        }
        return m_listeners;
    }

    public void publishDelete(Object entity) {
        for (Iterator i = getListeners().iterator(); i.hasNext();) {
            DaoEventListener listener = (DaoEventListener) i.next();
            listener.onDelete(entity);
        }
    }

    public void publishSave(Object entity) {
        for (Iterator i = getListeners().iterator(); i.hasNext();) {
            DaoEventListener listener = (DaoEventListener) i.next();
            listener.onSave(entity);
        }
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
        m_listeners = null;
    }
}
