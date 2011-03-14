package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.net.UnknownHostException;
import java.util.List;

import junit.framework.AssertionFailedError;
import junit.framework.TestCase;

import com.mongodb.BasicDBObject;
import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;
import com.mongodb.Mongo;
import com.mongodb.MongoException;

public class MongoTestCaseHelper {
    private static String m_host = "localhost";
    private static int m_port = 27017;
    private static Mongo m_mongoInstance;
    private static DBCollection m_collection;
    public final static String DOMAIN = "mydomain.org";

    public static DBCollection initMongo(String dbName, String collectionName) throws UnknownHostException,
            MongoException {
        if (m_mongoInstance == null) {
            m_mongoInstance = new Mongo(m_host, m_port);
        }
        //m_mongoInstance.dropDatabase(dbName);
        DB db = m_mongoInstance.getDB(dbName);
        m_collection = db.getCollection(collectionName);
        return m_collection;
    }

    public static void assertObjectPresent(DBObject ref) throws AssertionFailedError {
        TestCase.assertTrue(m_collection.find(ref).size() > 0);
    }

    public static void assertObjectWithIdPresent(String id) throws AssertionFailedError {
        DBObject ref = new BasicDBObject();
        ref.put("id", id);
        TestCase.assertEquals(1, m_collection.find(ref).size());
    }

    public static void assertObjectWithIdNotPresent(Object id) throws AssertionFailedError {
        DBObject ref = new BasicDBObject();
        ref.put("id", id);
        TestCase.assertEquals(0, m_collection.find(ref).size());
    }
    
    public static void assertCollectionItemsCount(DBObject ref, int count) {
        TestCase.assertTrue(m_collection.find(ref).size() == count);
    }

    public static void assertCollectionCount(int count) {
        TestCase.assertEquals(count, m_collection.find().count());
    }

    public static void assertObjectListFieldCount(String id, String listField, int count){
        DBObject ref = new BasicDBObject();
        ref.put("id", id);
        TestCase.assertEquals(1, m_collection.find(ref).size());
        DBObject obj = m_collection.findOne(ref);
        TestCase.assertTrue(obj.containsField(listField));
        TestCase.assertEquals(count, ((List<DBObject>)obj.get(listField)).size());
        
    }
    
    public static void assertObjectWithIdFieldValuePresent(Object id, String field, Object value) {
        DBObject ref = new BasicDBObject();
        ref.put("id", id);
        ref.put(field, value);
        TestCase.assertEquals(1, m_collection.find(ref).count());
    }
    
    public static void destroyAllDbs() {
        for (String db : m_mongoInstance.getDatabaseNames()) {
            m_mongoInstance.dropDatabase(db);
        }
    }
}
