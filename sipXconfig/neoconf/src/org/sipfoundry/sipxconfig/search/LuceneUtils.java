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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.lucene.index.IndexReader;
import org.apache.lucene.index.IndexWriter;
import org.apache.lucene.search.IndexSearcher;

public final class LuceneUtils {
    private static final Log LOG = LogFactory.getLog(LuceneUtils.class);
    private static final String ERROR = "ignoring error on close";

    private LuceneUtils() {
        // utility class - cannot create
    }

    public static void closeQuietly(IndexSearcher searcher) {
        if (searcher == null) {
            return;
        }
        try {
            searcher.close();
        } catch (IOException e) {
            LOG.error(ERROR, e);
        } catch (IllegalStateException e) {
            LOG.error(ERROR, e);
        }
    }

    public static void closeQuietly(IndexWriter writer) {
        if (writer == null) {
            return;
        }
        try {
            writer.close();
        } catch (IOException e) {
            LOG.error(ERROR, e);
        } catch (IllegalStateException e) {
            LOG.error(ERROR, e);
        }
    }

    public static void closeQuietly(IndexReader reader) {
        if (reader == null) {
            return;
        }
        try {
            reader.close();
        } catch (IOException e) {
            LOG.error(ERROR, e);
        } catch (IllegalStateException e) {
            LOG.error(ERROR, e);
        }
    }
}
