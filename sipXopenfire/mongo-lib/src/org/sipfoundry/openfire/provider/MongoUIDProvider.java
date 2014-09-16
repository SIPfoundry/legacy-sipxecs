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
