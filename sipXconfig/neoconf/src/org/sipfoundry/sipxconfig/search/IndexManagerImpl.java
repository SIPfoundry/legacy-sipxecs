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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.SessionFactory;
import org.springframework.orm.hibernate3.HibernateTemplate;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class IndexManagerImpl extends HibernateDaoSupport implements IndexManager {
    private static final Log LOG = LogFactory.getLog(IndexManagerImpl.class);

    private Indexer m_indexer;

    private BeanAdaptor m_beanAdaptor;

    private Class[] m_indexedClasses = DefaultBeanAdaptor.CLASSES;

    /**
     * Loads all entities to be indexed.
     */
    public void indexAll() {
        try {
            LOG.info("creating database index...");
            m_indexer.open();
            // load all classes that need to be indexed
            for (int i = 0; i < m_indexedClasses.length; i++) {
                m_beanAdaptor.setIndexedClasses(new Class[] {
                    m_indexedClasses[i]
                });
                getHibernateTemplate().loadAll(m_indexedClasses[i]);
            }
        } finally {
            m_indexer.close();
            LOG.info("index created");
        }
    }

    public void setIndexer(Indexer indexer) {
        m_indexer = indexer;
    }

    /**
     * In order for this code to work this has to be the same beanAdaptor that is used by Indexer
     */
    public void setBeanAdaptor(BeanAdaptor beanAdaptor) {
        m_beanAdaptor = beanAdaptor;
    }

    /**
     * Make sure that we always create a new session.
     *
     * We need to use our entity interceptor for indexing to work.
     */
    protected HibernateTemplate createHibernateTemplate(SessionFactory sessionFactory) {
        HibernateTemplate hibernate = super.createHibernateTemplate(sessionFactory);
        hibernate.setAlwaysUseNewSession(true);
        return hibernate;
    }
}
