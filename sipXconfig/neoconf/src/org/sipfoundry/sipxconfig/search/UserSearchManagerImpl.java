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

import java.util.List;

import org.apache.commons.collections.Transformer;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.lucene.index.Term;
import org.apache.lucene.search.BooleanClause;
import org.apache.lucene.search.BooleanQuery;
import org.apache.lucene.search.PrefixQuery;
import org.apache.lucene.search.Query;
import org.apache.lucene.search.TermQuery;
import org.sipfoundry.sipxconfig.common.User;

public class UserSearchManagerImpl implements UserSearchManager {
    private static final Log LOG = LogFactory.getLog(UserSearchManagerImpl.class);

    private SearchManager m_searchManager;

    public void setSearchManager(SearchManager searchManager) {
        m_searchManager = searchManager;
    }

    public List search(User template, int firstResult, int pageSize, Transformer transformer) {
        Query query = buildQueryForUser(template);
        return m_searchManager.search(query, firstResult, pageSize, transformer);
    }

    private Query buildQueryForUser(User user) {
        Term classTerm = new Term(Indexer.CLASS_FIELD, User.class.getName());
        TermQuery classQuery = new TermQuery(classTerm);

        BooleanQuery userQuery = new BooleanQuery();

        String firstName = user.getFirstName();
        if (StringUtils.isNotBlank(firstName)) {
            Query q = new PrefixQuery(new Term(User.FIRST_NAME_PROP, firstName.toLowerCase()));
            userQuery.add(q, BooleanClause.Occur.MUST);
        }

        String lastName = user.getLastName();
        if (StringUtils.isNotBlank(lastName)) {
            Query q = new PrefixQuery(new Term(User.LAST_NAME_PROP, lastName.toLowerCase()));
            userQuery.add(q, BooleanClause.Occur.MUST);
        }

        String userName = user.getUserName();
        if (StringUtils.isNotBlank(userName)) {
            userName = userName.toLowerCase();
            Query qName = new PrefixQuery(new Term(User.USER_NAME_PROP, userName));
            Query qAlias = new PrefixQuery(new Term("alias", userName));
            BooleanQuery aliasOrNameQuery = new BooleanQuery();
            aliasOrNameQuery.add(qName, BooleanClause.Occur.SHOULD);
            aliasOrNameQuery.add(qAlias, BooleanClause.Occur.SHOULD);

            userQuery.add(aliasOrNameQuery, BooleanClause.Occur.MUST);
        }

        // if no clauses were added just return class query
        if (userQuery.getClauses().length == 0) {
            return classQuery;
        }

        BooleanQuery query = new BooleanQuery();
        query.add(classQuery, BooleanClause.Occur.MUST);
        query.add(userQuery, BooleanClause.Occur.MUST);

        LOG.debug(query);
        return query;
    }
}
