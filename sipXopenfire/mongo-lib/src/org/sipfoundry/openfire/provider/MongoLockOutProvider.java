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

import org.bson.BasicBSONObject;
import org.jivesoftware.openfire.lockout.LockOutFlag;
import org.jivesoftware.openfire.provider.LockOutProvider;
import org.jivesoftware.util.StringUtils;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBCursor;
import com.mongodb.DBObject;

public class MongoLockOutProvider extends BaseMongoProvider implements LockOutProvider {
    private static final String COLLECTION_NAME = "ofUserFlag";

    public MongoLockOutProvider() {
        setDefaultCollectionName(COLLECTION_NAME);
        DBCollection lockoutCollection = getDefaultCollection();
        DBObject index = new BasicDBObject();

        index.put("username", 1);
        index.put("name", 1);
        lockoutCollection.ensureIndex(index);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public LockOutFlag getDisabledStatus(String username) {
        LockOutFlag flag = null;
        DBCollection lockoutCollection = getDefaultCollection();
        DBObject query = new BasicDBObject();

        query.put("username", username);
        // name of the lockout flag
        query.put("name", "lockout");

        DBCursor cursor = lockoutCollection.find(query);

        if (cursor.hasNext()) {
            DBObject line = cursor.next();
            BasicBSONObject obj = new BasicBSONObject();
            obj.putAll(line);
            long start = obj.getLong("startTime");
            long end = obj.getLong("endTime");

            flag = new LockOutFlag(username, new Date(start), new Date(end));
        }

        cursor.close();

        return flag;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setDisabledStatus(LockOutFlag flag) {
        DBCollection lockoutCollection = getDefaultCollection();
        DBObject toInsert = new BasicDBObject();

        toInsert.put("username", flag.getUsername());
        toInsert.put("name", "lockout");
        toInsert.put("startTime", StringUtils.dateToMillis(flag.getStartTime()));
        toInsert.put("endTime", StringUtils.dateToMillis(flag.getEndTime()));

        lockoutCollection.insert(toInsert);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void unsetDisabledStatus(String username) {
        DBCollection lockoutCollection = getDefaultCollection();
        DBObject toDelete = new BasicDBObject();

        toDelete.put("username", username);

        lockoutCollection.remove(toDelete);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isReadOnly() {
        return false;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isDelayedStartSupported() {
        return true;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isTimeoutSupported() {
        return true;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean shouldNotBeCached() {
        return false;
    }
}
