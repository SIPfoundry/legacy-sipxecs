/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;

import org.sipfoundry.sipxconfig.common.Replicable;

import static org.sipfoundry.commons.mongo.MongoConstants.ID;
import static org.sipfoundry.commons.mongo.MongoConstants.IDENTITY;
import static org.sipfoundry.commons.mongo.MongoConstants.VALID_USER;

public class DataSetGenerator extends AbstractDataSetGenerator {

    // We can safely assume that every replicable entity is a beanwithid
    // We can treat special cases separately
    public DBObject findOrCreate(Replicable entity) {
        DBCollection collection = getDbCollection();
        String id = getEntityId(entity);

        DBObject search = new BasicDBObject();
        search.put(ID, id);
        // DBObject cursor = collection.findOne(search);
        DBObject top = collection.findOne(search);
        if (top == null) {
            top = new BasicDBObject();
            top.put(ID, id);
        }
        if (entity.getIdentity(getSipDomain()) != null) {
            top.put(IDENTITY, entity.getIdentity(getSipDomain()));
        }
        for (String key : entity.getMongoProperties(getSipDomain()).keySet()) {
            top.put(key, entity.getMongoProperties(getSipDomain()).get(key));
        }
        if (entity.isValidUser()) {
            top.put(VALID_USER, true);
        }
        return top;
    }

    @Override
    public void generate(Replicable entity, DBObject top) {
        // empty implementation
    }

    @Override
    protected DataSet getType() {
        // empty implementation
        return null;
    }

}
