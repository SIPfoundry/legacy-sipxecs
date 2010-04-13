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

public abstract class EntityDeleteListener<T> implements DaoEventListener {
    private Class<T> m_klass;

    public EntityDeleteListener(Class<T> klass) {
        m_klass = klass;
    }

    public void onDelete(Object entity) {
        if (m_klass.isAssignableFrom(entity.getClass())) {
            onEntityDelete((T) entity);
        }
    }

    public void onSave(Object entity_) {
    }

    protected abstract void onEntityDelete(T group);
}
