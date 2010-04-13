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
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.apache.commons.collections.Transformer;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.document.Document;
import org.apache.lucene.index.Term;
import org.apache.lucene.queryParser.ParseException;
import org.apache.lucene.queryParser.QueryParser;
import org.apache.lucene.search.BooleanClause;
import org.apache.lucene.search.BooleanQuery;
import org.apache.lucene.search.Hits;
import org.apache.lucene.search.IndexSearcher;
import org.apache.lucene.search.PrefixQuery;
import org.apache.lucene.search.Query;
import org.apache.lucene.search.Sort;
import org.apache.lucene.search.SortField;
import org.apache.lucene.search.TermQuery;
import org.sipfoundry.sipxconfig.search.BeanAdaptor.Identity;

public class SearchManagerImpl implements SearchManager {
    private static final Log LOG = LogFactory.getLog(SearchManagerImpl.class);

    private IndexSource m_indexSource;

    private Analyzer m_analyzer;

    private BeanAdaptor m_beanAdaptor;

    public void setIndexSource(IndexSource indexSource) {
        m_indexSource = indexSource;
    }

    public void setAnalyzer(Analyzer analyzer) {
        m_analyzer = analyzer;
    }

    public void setBeanAdaptor(BeanAdaptor beanAdaptor) {
        m_beanAdaptor = beanAdaptor;
    }

    public List search(String queryText, Transformer transformer) {
        try {
            Query query = parseUserQuery(queryText);
            return search(query, 0, -1, null, transformer);
        } catch (ParseException e) {
            LOG.info(e.getMessage());
        }
        return Collections.EMPTY_LIST;
    }

    public List search(Class entityClass, String queryText, Transformer transformer) {
        return search(entityClass, queryText, 0, -1, null, false, transformer);
    }

    public List search(Query query, int firstResult, int pageSize, Transformer transformer) {
        return search(query, firstResult, pageSize, null, transformer);
    }

    private List search(Query query, int firstItem, int pageSize, Sort sort,
            Transformer transformer) {
        IndexSearcher searcher = null;
        try {
            searcher = m_indexSource.getSearcher();
            Hits hits = searcher.search(query, sort);
            List found = hits2beans(hits, transformer, firstItem, pageSize);
            return found;
        } catch (IOException e) {
            LOG.error("search by user query error", e);
        } finally {
            LuceneUtils.closeQuietly(searcher);
        }
        return Collections.EMPTY_LIST;
    }

    private List hits2beans(Hits hits, Transformer transformer, int firstItem, int pageSize)
        throws IOException {
        final int hitCount = hits.length();
        List results = new ArrayList(hitCount);
        // if (transformer != null) {
        // results = ListUtils.predicatedList(results, NotNullPredicate.INSTANCE);
        // results = ListUtils.transformedList(results, transformer);
        // }

        int from = firstItem < 0 ? 0 : firstItem;
        int to = pageSize < 0 ? hitCount : Math.min(hitCount, firstItem + pageSize);
        for (int i = from; i < to; i++) {
            Document document = hits.doc(i);
            Identity identity = m_beanAdaptor.getBeanIdentity(document);
            if (identity == null) {
                continue;
            }
            if (transformer != null) {
                Object bean = transformer.transform(identity);
                if (bean != null) {
                    results.add(bean);
                }
            } else {
                results.add(identity);
            }
        }

        return results;
    }

    /**
     * Return prefix queries if user only enters single field, otherwise use query parser
     *
     * @param queryText
     * @return newly created query object
     */
    Query parseUserQuery(String queryText) throws ParseException {
        QueryParser parser = new QueryParser(Indexer.DEFAULT_FIELD, m_analyzer);
        Query query = parser.parse(queryText);
        if (query instanceof TermQuery) {
            TermQuery termQuery = (TermQuery) query;
            return new PrefixQuery(termQuery.getTerm());
        }
        return query;
    }

    public List search(Class entityClass, String queryText, int firstResult, int pageSize,
            String[] sortFields, boolean orderAscending, Transformer transformer) {
        try {
            Query userQuery = parseUserQuery(queryText);
            Term classTerm = new Term(Indexer.CLASS_FIELD, entityClass.getName());
            TermQuery classQuery = new TermQuery(classTerm);
            BooleanQuery query = new BooleanQuery();
            query.add(classQuery, BooleanClause.Occur.MUST);
            query.add(userQuery, BooleanClause.Occur.MUST);
            Sort sort = createSortFromFields(sortFields, orderAscending);
            return search(query, firstResult, pageSize, sort, transformer);
        } catch (ParseException e) {
            LOG.info(e.getMessage());
        }

        return Collections.EMPTY_LIST;
    }

    private Sort createSortFromFields(String[] sortFields, boolean orderAscending) {
        if (sortFields == null || sortFields.length == 0) {
            return null;
        }
        SortField[] sfs = new SortField[sortFields.length];
        for (int i = 0; i < sfs.length; i++) {
            sfs[i] = new SortField(sortFields[i], SortField.STRING, !orderAscending);
        }
        return new Sort(sfs);
    }
}
