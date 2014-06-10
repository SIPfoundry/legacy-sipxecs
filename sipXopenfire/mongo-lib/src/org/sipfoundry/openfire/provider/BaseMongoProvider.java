/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
