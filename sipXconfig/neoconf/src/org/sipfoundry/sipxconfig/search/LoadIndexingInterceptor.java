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

/**
 * This is used to indexing on load
 */
public class LoadIndexingInterceptor extends SpringHibernateInstantiator {
    private Indexer m_indexer;

    public void setIndexer(Indexer indexer) {
        m_indexer = indexer;
    }

    public boolean onLoad(Object entity, Serializable id, Object[] state, String[] propertyNames,
            Type[] types) {
        m_indexer.indexBean(entity, id, state, propertyNames, types, true);
        return false;
    }
}
