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
import org.apache.lucene.search.Query;

public interface SearchManager {
    public static final String CONTEXT_BEAN_NAME = "searchManager";

    List search(String query, Transformer transformer);

    List search(Class entityClass, String query, Transformer transformer);

    /**
     * Search for an indexed item using class and user entered query.
     *
     * @param entityClass class of the entity we are searching for
     * @param query lucene query that is ANDed with entityClass query
     * @param firstResult index of the first found result
     * @param pageSize
     * @param sort field that the result is sorted
     * @param orderAscending if true use ascending order when sorting, if false use descending
     *        order
     * @param transformer commons collections compatible transformed - if provided it'll be
     *        applied to each found item before the results are returned
     * @return collection of BeanAdapter.Identity objects - if no transformer provided,
     *         transformed collection if there was a transformer
     */
    List search(Class entityClass, String query, int firstResult, int pageSize, String[] sort,
            boolean orderAscending, Transformer transformer);

    List search(Query query, int firstResult, int pageSize, Transformer transformer);
}
