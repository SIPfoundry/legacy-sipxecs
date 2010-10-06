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

import java.io.File;
import java.io.IOException;

import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.index.IndexReader;
import org.apache.lucene.index.IndexWriter;
import org.apache.lucene.search.IndexSearcher;
import org.apache.lucene.store.Directory;
import org.apache.lucene.store.FSDirectory;

/**
 * Naive implementation, always return FS directory, do not try to cache or optimize anything,
 * recreate if it does not exist
 *
 */
public class SimpleIndexSource implements IndexSource {
    private File m_indexDirectory;

    private boolean m_createDirectory;

    private boolean m_createIndex;

    private Analyzer m_analyzer;

    private int m_maxFieldLength = IndexWriter.DEFAULT_MAX_FIELD_LENGTH;

    private int m_minMergeDocs = IndexWriter.DEFAULT_MAX_BUFFERED_DOCS;

    private Directory getDirectory() {
        try {
            Directory directory = createDirectory(m_indexDirectory);
            m_createDirectory = false;
            return directory;
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Overwrite to create different directory
     *
     * @throws IOException
     */
    protected Directory createDirectory(File file) throws IOException {
        return FSDirectory.open(file);
    }

    public void setIndexDirectoryName(String indexDirectoryName) {
        m_indexDirectory = new File(indexDirectoryName);
        m_createDirectory = !m_indexDirectory.exists();
        m_createIndex = m_createDirectory;
    }

    public void setAnalyzer(Analyzer analyzer) {
        m_analyzer = analyzer;
    }

    public void setMinMergeDocs(int minMergeDocs) {
        m_minMergeDocs = minMergeDocs;
    }

    public void setMaxFieldLength(int maxFieldLength) {
        m_maxFieldLength = maxFieldLength;
    }

    public IndexReader getReader() throws IOException {
        ensureIndexExists();
        return IndexReader.open(getDirectory(), false);
    }

    public IndexWriter getWriter(boolean createNew) throws IOException {
        IndexWriter writer = new IndexWriter(getDirectory(), m_analyzer, createNew
                || m_createIndex, IndexWriter.MaxFieldLength.LIMITED);
        writer.setMaxFieldLength(m_maxFieldLength);
        writer.setMaxBufferedDocs(m_minMergeDocs);
        m_createIndex = false;
        return writer;
    }

    public IndexSearcher getSearcher() throws IOException {
        ensureIndexExists();
        return new IndexSearcher(getDirectory());
    }

    private void ensureIndexExists() throws IOException {
        if (m_createIndex) {
            IndexWriter writer = getWriter(false);
            writer.close();
        }
    }
}
