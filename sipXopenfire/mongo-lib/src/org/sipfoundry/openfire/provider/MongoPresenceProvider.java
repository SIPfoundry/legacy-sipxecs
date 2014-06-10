/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
