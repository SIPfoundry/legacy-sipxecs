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

import java.util.Date;

import org.jivesoftware.openfire.provider.PresenceProvider;
import org.jivesoftware.util.StringUtils;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBCursor;
import com.mongodb.DBObject;
import com.mongodb.WriteConcern;

public class MongoPresenceProvider extends BaseMongoProvider implements PresenceProvider {
    private static final String COLLECTION_NAME = "ofPresence";

    public MongoPresenceProvider() {
        setDefaultCollectionName(COLLECTION_NAME);
        DBCollection presenceCollection = getDefaultCollection();

        presenceCollection.ensureIndex("username");
    }

    @Override
    public void deleteOfflinePresenceFromDB(String username) {
        DBCollection presenceCollection = getDefaultCollection();
        DBObject toRemove = new BasicDBObject();
        toRemove.put("username", username);

        presenceCollection.remove(toRemove);
    }

    @Override
    public void insertOfflinePresenceIntoDB(String username, String offlinePresence, Date offlinePresenceDate) {
        DBCollection presenceCollection = getDefaultCollection();
        DBObject query = new BasicDBObject();
        query.put("username", username);

        DBObject toInsert = new BasicDBObject();
        toInsert.put("username", username);
        toInsert.put("offlinePresence", offlinePresence);
        String inMillis = StringUtils.dateToMillis(offlinePresenceDate);
        toInsert.put("offlinePresenceDate", inMillis);

        presenceCollection.update(query, toInsert, true, false, WriteConcern.NONE);
    }

    @Override
    public TimePresence loadOfflinePresence(String username) {
        TimePresence tp;
        DBCollection presenceCollection = getDefaultCollection();
        DBObject toFind = new BasicDBObject();
        toFind.put("username", username);
        DBCursor cursor = presenceCollection.find(toFind);

        if (cursor.hasNext()) {
            DBObject entry = cursor.next();

            String lastActivity = (String) entry.get("offlinePresenceDate");
            String presence = (String) entry.get("offlinePresence");

            tp = new TimePresence(Long.valueOf(lastActivity), presence);
        } else {
            tp = new TimePresence(NULL_LONG, "NULL");
        }

        cursor.close();

        return tp;
    }

}
