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

import org.apache.lucene.document.Document;
import org.apache.lucene.index.IndexWriter;
import org.hibernate.type.Type;

/**
 * Used to create index - optimized for indexing large sets.
 *
 * No support for removing beans.
 */
public class BulkIndexer implements Indexer {
    private IndexSource m_indexSource;

    private BeanAdaptor m_beanAdaptor;

    private IndexWriter m_writer;

    public void indexBean(Object bean, Serializable id, Object[] state, String[] fieldNames,
            Type[] types, boolean newInstance_) {
        try {
            Document document = new Document();
            if (m_beanAdaptor.documentFromBean(document, bean, id, state, fieldNames, types)) {
                m_writer.addDocument(document);
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void removeBean(Object bean_, Serializable id_) {
        throw new UnsupportedOperationException("only used to add new beans");
    }

    public void open() {
        try {
            m_writer = m_indexSource.getWriter(true);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void close() {
        try {
            m_writer.optimize();
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            LuceneUtils.closeQuietly(m_writer);
        }
    }

    public void setIndexSource(IndexSource indexSource) {
        m_indexSource = indexSource;
    }

    public void setBeanAdaptor(BeanAdaptor beanAdaptor) {
        m_beanAdaptor = beanAdaptor;
    }
}
