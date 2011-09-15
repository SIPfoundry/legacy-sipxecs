/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.net.UnknownHostException;
import java.util.List;

import org.sipfoundry.commons.mongo.MongoAccessController;

import junit.framework.TestCase;

import com.mongodb.BasicDBObject;
import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;
import com.mongodb.util.JSON;

public final class MongoTestCaseHelper {
    public static final String DOMAIN = "mydomain.org";
    public static final String ID = "_id";
    private static DBCollection s_collection;

    private MongoTestCaseHelper() {
    }

    public static DBCollection initMongo(String dbName, String collectionName) throws UnknownHostException {
        dropDb(dbName);
        DB db = MongoAccessController.INSTANCE.getDatabase(dbName);
        s_collection = db.getCollection(collectionName);
        return s_collection;
    }

    public static void assertObjectPresent(DBObject ref) {
        TestCase.assertTrue(s_collection.find(ref).size() > 0);
    }

    public static void assertObjectWithIdPresent(String id) {
        DBObject ref = new BasicDBObject();
        ref.put(ID, id);
        TestCase.assertEquals(1, s_collection.find(ref).size());
    }

    public static void assertObjectWithIdNotPresent(Object id) {
        DBObject ref = new BasicDBObject();
        ref.put(ID, id);
        TestCase.assertEquals(0, s_collection.find(ref).size());
    }

    public static void assertCollectionItemsCount(DBObject ref, int count) {
        TestCase.assertTrue(s_collection.find(ref).size() == count);
    }

    public static void assertCollectionCount(int count) {
        TestCase.assertEquals(count, s_collection.find().count());
    }

    public static void assertObjectListFieldCount(String id, String listField, int count) {
        DBObject ref = new BasicDBObject();
        ref.put(ID, id);
        TestCase.assertEquals(1, s_collection.find(ref).size());
        DBObject obj = s_collection.findOne(ref);
        TestCase.assertTrue(obj.containsField(listField));
        TestCase.assertEquals(count, ((List<DBObject>) obj.get(listField)).size());

    }

    public static void assertObjectWithIdFieldValuePresent(Object id, String field, Object value) {
        DBObject ref = new BasicDBObject();
        ref.put(ID, id);
        ref.put(field, value);
        TestCase.assertEquals(1, s_collection.find(ref).count());
    }

    public static void assertObjectWithIdFieldValueNotPresent(Object id, String field, Object value) {
        DBObject ref = new BasicDBObject();
        ref.put(ID, id);
        ref.put(field, value);
        TestCase.assertEquals(0, s_collection.find(ref).count());
    }

    public static void insert(DBObject dbo) {
        s_collection.insert(dbo);
    }

    public static void insertJson(String... jsons) {
        for (String json : jsons) {
            s_collection.save((DBObject) JSON.parse(json));
        }
    }

    public static void dropDb(String db) throws UnknownHostException {
        MongoAccessController.INSTANCE.dropDatabase(db);
    }
}
