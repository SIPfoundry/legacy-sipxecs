/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 */
package org.sipfoundry.openfire.provider;

import org.jivesoftware.openfire.provider.UIDProvider;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;

public class MongoUIDProvider extends BaseMongoProvider implements UIDProvider {
    private static final String COLLECTION_NAME = "ofId";

    public MongoUIDProvider() {
        setDefaultCollectionName(COLLECTION_NAME);
        DBCollection idCollection = getDefaultCollection();
        DBObject index = new BasicDBObject();

        index.put("idType", 1);

        idCollection.ensureIndex(index);
    }

    @Override
    public long[] getNextBlock(int type, int blockSize) {
        long[] result = new long[2]; // we just return the min and max ids
        DBCollection idCollection = getDefaultCollection();
        DBObject query = new BasicDBObject();

        query.put("idType", type);

        DBObject idObj = idCollection.findOne(query);

        if (idObj != null) {
            result[0] = (Long) idObj.get("id");
        } else {
            result[0] = 1;
        }
        result[1] = result[0] + blockSize;

        DBObject update = new BasicDBObject();
        update.put("idType", type);
        update.put("id", result[1]);

        // update if exists, otherwise insert
        idCollection.findAndModify(query, null, null, false, new BasicDBObject("$set", update), false, true);

        return result;
    }

}
