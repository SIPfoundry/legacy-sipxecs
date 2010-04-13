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

import org.hibernate.type.Type;
import org.sipfoundry.sipxconfig.common.SpringHibernateInstantiator;

public class IndexingInterceptor extends SpringHibernateInstantiator {
    private Indexer m_indexer;

    public void setIndexer(Indexer indexer) {
        m_indexer = indexer;
    }

    public boolean onSave(Object entity, Serializable id, Object[] state, String[] propertyNames,
            Type[] types) {
        m_indexer.indexBean(entity, id, state, propertyNames, types, true);
        return false;
    }

    public void onDelete(Object entity, Serializable id, Object[] state_,
            String[] propertyNames_, Type[] types_) {
        m_indexer.removeBean(entity, id);
    }

    public boolean onFlushDirty(Object entity, Serializable id, Object[] currentState,
            Object[] previousState_, String[] propertyNames, Type[] types) {
        m_indexer.indexBean(entity, id, currentState, propertyNames, types, false);
        return false;
    }
}
