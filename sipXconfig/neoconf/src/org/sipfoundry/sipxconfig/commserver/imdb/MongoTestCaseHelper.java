/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.commserver.imdb;

import java.util.List;

import junit.framework.TestCase;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBCursor;
import com.mongodb.DBObject;
import com.mongodb.util.JSON;

public final class MongoTestCaseHelper {
    public static final String DOMAIN = "mydomain.org";
    public static final String ID = "_id";
    public static final String EXCEPTION = "fields and values do not match (they have different legths)";

    private MongoTestCaseHelper() {
    }

    public static final void assertObjectPresent(DBCollection collection, DBObject ref) {
        TestCase.assertTrue(collection.find(ref).size() > 0);
    }

    public static final void assertObjectNotPresent(DBCollection collection, DBObject ref) {
        TestCase.assertTrue(collection.find(ref).size() == 0);
    }

    public static final void assertObjectWithIdPresent(DBCollection collection, String id) {
        DBObject ref = new BasicDBObject();
        ref.put(ID, id);
        TestCase.assertEquals(1, collection.find(ref).size());
    }

    public static final void assertObjectWithIdNotPresent(DBCollection collection, Object id) {
        DBObject ref = new BasicDBObject();
        ref.put(ID, id);
        TestCase.assertEquals(0, collection.find(ref).size());
    }

    public static final void assertCollectionItemsCount(DBCollection collection, DBObject ref, int count) {
        TestCase.assertTrue(collection.find(ref).size() == count);
    }

    public static final void assertCollectionCount(DBCollection collection, int count) {
        TestCase.assertEquals(count, collection.find().count());
    }

    public static final void assertObjectListFieldCount(DBCollection collection, String id, String listField,
            int count) {
        DBObject ref = new BasicDBObject();
        ref.put(ID, id);
        TestCase.assertEquals(1, collection.find(ref).size());
        DBObject obj = collection.findOne(ref);
        TestCase.assertTrue(obj.containsField(listField));
        TestCase.assertEquals(count, ((List<DBObject>) obj.get(listField)).size());

    }

    public static final void assertObjectWithFieldsValuesPresent(DBCollection collection, String[] fields,
            Object[] values) {
        if (fields.length != values.length) {
            throw new RuntimeException(EXCEPTION);
        }
        DBObject ref = new BasicDBObject();
        for (int i = 0; i < fields.length; i++) {
            ref.put(fields[i], values[i]);
        }
        TestCase.assertEquals(1, collection.find(ref).count());
    }

    public static final void assertObjectWithFieldsValuesNotPresent(DBCollection collection, String[] fields,
            Object[] values) {
        if (fields.length != values.length) {
            throw new RuntimeException(EXCEPTION);
        }
        DBObject ref = new BasicDBObject();
        for (int i = 0; i < fields.length; i++) {
            ref.put(fields[i], values[i]);
        }
        TestCase.assertEquals(0, collection.find(ref).count());
    }

    public static final void assertObjectWithIdFieldValuePresent(DBCollection collection, Object id, String field,
            Object value) {
        DBObject ref = new BasicDBObject();
        ref.put(ID, id);
        ref.put(field, value);
        TestCase.assertEquals(1, collection.find(ref).count());
    }

    public static final void assertObjectWithIdFieldValueNotPresent(DBCollection collection, Object id,
            String field, Object value) {
        DBObject ref = new BasicDBObject();
        ref.put(ID, id);
        ref.put(field, value);
        TestCase.assertEquals(0, collection.find(ref).count());
    }

    public static final void insert(DBCollection collection, DBObject dbo) {
        collection.insert(dbo);
    }

    public static final void insertJson(DBCollection collection, String... jsons) {
        for (String json : jsons) {
            collection.save((DBObject) JSON.parse(json));
        }
    }

    public static final void assertJson(DBCursor actual, String expectedJson) {
        String actualJson = JSON.serialize(actual.toArray());
        TestCase.assertEquals(expectedJson, actualJson);
    }
}
