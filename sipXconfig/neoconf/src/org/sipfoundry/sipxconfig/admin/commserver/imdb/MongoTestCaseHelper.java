/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.commons.mongo.MongoDbTemplate;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;
import com.mongodb.util.JSON;

public final class MongoTestCaseHelper {

    public static final String DOMAIN = "mydomain.org";
    public static final String ID = "_id";
    private MongoDbTemplate m_dbt = new MongoDbTemplate();
    private String m_collection;

    public MongoTestCaseHelper() {
        m_dbt.setName("test");
        m_collection = "entity";
    }

    public MongoTestCaseHelper(String dbname, String collectionName) {
        m_dbt.setName(dbname);
        m_collection = collectionName;
    }

    public DBCollection getCollection() {
        return m_dbt.getDb().getCollection(m_collection);
    }

    public void assertObjectPresent(DBObject ref) {
        TestCase.assertTrue(getCollection().find(ref).size() > 0);
    }

    public void assertObjectWithIdPresent(String id) {
        DBObject ref = new BasicDBObject();
        ref.put(ID, id);
        TestCase.assertEquals(1, getCollection().find(ref).size());
    }

    public void assertObjectWithIdNotPresent(Object id) {
        DBObject ref = new BasicDBObject();
        ref.put(ID, id);
        TestCase.assertEquals(0, getCollection().find(ref).size());
    }

    public void assertCollectionItemsCount(DBObject ref, int count) {
        TestCase.assertTrue(getCollection().find(ref).size() == count);
    }

    public void assertCollectionCount(int count) {
        TestCase.assertEquals(count, getCollection().find().count());
    }

    public void assertObjectListFieldCount(String id, String listField, int count) {
        DBObject ref = new BasicDBObject();
        ref.put(ID, id);
        TestCase.assertEquals(1, getCollection().find(ref).size());
        DBObject obj = getCollection().findOne(ref);
        TestCase.assertTrue(obj.containsField(listField));
        TestCase.assertEquals(count, ((List<DBObject>) obj.get(listField)).size());

    }

    public void assertObjectWithIdFieldValuePresent(Object id, String field, Object value) {
        DBObject ref = new BasicDBObject();
        ref.put(ID, id);
        ref.put(field, value);
        TestCase.assertEquals(1, getCollection().find(ref).count());
    }

    public void assertObjectWithIdFieldValueNotPresent(Object id, String field, Object value) {
        DBObject ref = new BasicDBObject();
        ref.put(ID, id);
        ref.put(field, value);
        TestCase.assertEquals(0, getCollection().find(ref).count());
    }

    public void insert(DBObject dbo) {
        getCollection().insert(dbo);
    }

    public void insertJson(String... jsons) {
        for (String json : jsons) {
            getCollection().save((DBObject) JSON.parse(json));
        }
    }

    public void dropDb() {
        m_dbt.getMongo().dropDatabase(m_dbt.getName());
    }

    public MongoDbTemplate getDbTemplate() {
        return m_dbt;
    }

    public void setDbTemplate(MongoDbTemplate dbt) {
        m_dbt = dbt;
    }
}
