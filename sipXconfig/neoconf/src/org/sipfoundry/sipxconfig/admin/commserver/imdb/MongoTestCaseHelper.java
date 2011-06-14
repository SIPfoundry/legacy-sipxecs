/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.net.UnknownHostException;
import java.util.List;

import junit.framework.TestCase;

import com.mongodb.BasicDBObject;
import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;
import com.mongodb.Mongo;


public final class MongoTestCaseHelper {
    public static final String DOMAIN = "mydomain.org";
    public static final String ID = "_id";
    private static String s_host = "localhost";
    private static int s_port = 27017;
    private static Mongo s_mongoInstance;
    private static DBCollection s_collection;

    private MongoTestCaseHelper() {
    }

    public static DBCollection initMongo(String dbName, String collectionName) throws UnknownHostException {
        if (s_mongoInstance == null) {
            s_mongoInstance = new Mongo(s_host, s_port);
        }
        //m_mongoInstance.dropDatabase(dbName);
        DB db = s_mongoInstance.getDB(dbName);
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

    public static void dropDb(String db) {
        s_mongoInstance.dropDatabase(db);
    }
}
