package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;

import java.util.List;

import junit.framework.AssertionFailedError;
import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.domain.DomainManager;

import com.mongodb.BasicDBObject;
import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;
import com.mongodb.Mongo;

public class MongoTestCase extends TestCase {
    private String m_host = "localhost";
    private int m_port = 27017;
    private Mongo m_mongoInstance;
    private static DBCollection m_collection;
    public final static String DBNAME = "imdb";
    public final static String DOMAIN = "mydomain.org";
    private CoreContext m_coreContext;
    private DomainManager m_dm;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        if (m_mongoInstance == null) {
            m_mongoInstance = new Mongo(m_host, m_port);
        }
        DB datasetDb = m_mongoInstance.getDB(DBNAME);
        m_collection = datasetDb.getCollection(DOMAIN);

        m_coreContext = createMock(CoreContext.class);
        m_coreContext.getDomainName();
        expectLastCall().andReturn(DOMAIN).anyTimes();
        m_coreContext.getAuthorizationRealm();
        expectLastCall().andReturn(DOMAIN).anyTimes();
        
        m_dm = createMock(DomainManager.class);
        m_dm.getDomainName();
        expectLastCall().andReturn(DOMAIN).anyTimes();
    }

    static void assertObjectPresent(DBObject ref) throws AssertionFailedError {
        assertTrue(m_collection.find(ref).size() > 0);
    }

    static void assertObjectWithIdPresent(String id) throws AssertionFailedError {
        DBObject ref = new BasicDBObject();
        ref.put("id", id);
        assertEquals(1, m_collection.find(ref).size());
    }

    static void assertObjectWithIdNotPresent(String id) throws AssertionFailedError {
        DBObject ref = new BasicDBObject();
        ref.put("id", id);
        assertEquals(0, m_collection.find(ref).size());
    }
    
    static void assertCollectionItemsCount(DBObject ref, int count) {
        assertTrue(m_collection.find(ref).size() == count);
    }

    static void assertCollectionCount(int count) {
        assertEquals(count, m_collection.find().count());
    }

    static void assertObjectListFieldCount(String id, String listField, int count){
        DBObject ref = new BasicDBObject();
        ref.put("id", id);
        assertEquals(1, m_collection.find(ref).size());
        DBObject obj = m_collection.findOne(ref);
        assertTrue(obj.containsField(listField));
        assertEquals(count, ((List<DBObject>)obj.get(listField)).size());
        
    }
    
    static void assertObjectWithIdFieldValuePresent(String id, String field, String value) {
        DBObject ref = new BasicDBObject();
        ref.put("id", id);
        ref.put(field, value);
        assertEquals(1, m_collection.find(ref).count());
    }

    public DBCollection getCollection() {
        return m_collection;
    }

    public CoreContext getCoreContext() {
        return m_coreContext;
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        m_mongoInstance.dropDatabase(DBNAME);
    }

    public DomainManager getDomainManager() {
        return m_dm;
    }

}
