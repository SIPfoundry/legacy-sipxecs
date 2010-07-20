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

import java.io.IOException;
import java.io.Serializable;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.lucene.document.Document;
import org.apache.lucene.index.IndexReader;
import org.apache.lucene.index.IndexWriter;
import org.apache.lucene.index.Term;
import org.hibernate.type.Type;

public class FastIndexer implements Indexer {
    private static final Log LOG = LogFactory.getLog(FastIndexer.class);
    private IndexSource m_indexSource;

    private BeanAdaptor m_beanAdaptor;

    public void indexBean(Object bean, Serializable id, Object[] state, String[] fieldNames,
            Type[] types, boolean newInstance) {
        Document document = new Document();
        if (!m_beanAdaptor.documentFromBean(document, bean, id, state, fieldNames, types)) {
            return;
        }
        if (!newInstance) {
            internalRemoveBean(bean, id);
        }
        addBean(document);
    }

    public void removeBean(Object bean, Serializable id) {
        // only remove beans that are index-able
        if (m_beanAdaptor.indexClass(new Document(), bean.getClass())) {
            internalRemoveBean(bean, id);
        }
    }

    private synchronized void addBean(Document document) {
        IndexWriter writer = null;
        try {
            writer = m_indexSource.getWriter(false);
            writer.addDocument(document);
        } catch (IOException e) {
            LOG.error(e);
        } finally {
            LuceneUtils.closeQuietly(writer);
        }
    }

    private synchronized void internalRemoveBean(Object bean, Serializable id) {
        IndexReader reader = null;
        try {
            reader = m_indexSource.getReader();
            Term idTerm = m_beanAdaptor.getIdentityTerm(bean, id);
            reader.deleteDocuments(idTerm);
        } catch (IOException e) {
            LOG.error(e);
        } finally {
            LuceneUtils.closeQuietly(reader);
        }
    }

    public void setBeanAdaptor(BeanAdaptor beanAdaptor) {
        m_beanAdaptor = beanAdaptor;
    }

    public void setIndexSource(IndexSource indexSource) {
        m_indexSource = indexSource;
    }

    public void open() {
        // intentionally empty
    }

    public void close() {
        // intentionally empty
    }
}
