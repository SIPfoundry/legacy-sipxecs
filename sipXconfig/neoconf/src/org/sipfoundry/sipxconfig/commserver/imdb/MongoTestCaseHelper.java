/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 * $
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

    private MongoTestCaseHelper() {
    }

    public static final void assertObjectPresent(DBCollection collection, DBObject ref) {
        TestCase.assertTrue(collection.find(ref).size() > 0);
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
