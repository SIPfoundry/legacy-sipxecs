/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.openfire.provider;

import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;

import com.mongodb.DB;
import com.mongodb.DBCollection;

/**
 * Root class for mongo providers. Provides common methods for collections retrieval.
 */
public abstract class BaseMongoProvider {
    /**
     * When a provider uses only one collection or has a more frequently used collection, it
     * should set its name as default for simplified retrieval.
     */
    private String m_defaultCollectionName;

    /**
     * Returns the default collection for this class
     *
     * @return {@link DBCollection}
     */
    protected DBCollection getDefaultCollection() {
        return getCollection(m_defaultCollectionName);
    }

    /**
     * Returns the collection having the specified name
     *
     * @param collectionName Name of the collection to retrieve.
     * @return {@link DBCollection}
     */
    protected static DBCollection getCollection(String collectionName) {
        DB db = UnfortunateLackOfSpringSupportFactory.getOpenfiredb();

        return db.getCollection(collectionName);
    }

    /**
     * @param collectionName Name of the default collection.
     */
    protected void setDefaultCollectionName(String collectionName) {
        m_defaultCollectionName = collectionName;
    }
}
