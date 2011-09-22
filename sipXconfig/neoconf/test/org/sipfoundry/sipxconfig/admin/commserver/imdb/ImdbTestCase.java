package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.List;

import org.sipfoundry.commons.mongo.MongoDbTemplate;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.permission.PermissionManager;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;
import com.mongodb.util.JSON;


public class ImdbTestCase extends IntegrationTestCase {
    public static final String DOMAIN = "example.org";
    public static final String ID = "_id";
    private CoreContext m_coreContext;
    private MongoDbTemplate m_imdb;    
    private PermissionManager m_permissionManager;
    private DomainManager m_domainManager;

    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        m_imdb.drop();
    }

    public DBCollection getEntityCollection() {
        DBCollection entity = m_imdb.getDb().getCollection("entity");
        return entity;
    }

    public void assertObjectPresent(DBObject ref) {
        assertNotNull(getEntityCollection().findOne(ref));
    }

    public void assertObjectWithIdPresent(String id) {
        DBObject ref = new BasicDBObject();
        ref.put(ID, id);
        assertEquals(1, getEntityCollection().find(ref).size());
    }

    public void assertObjectWithIdNotPresent(Object id) {
        DBObject ref = new BasicDBObject();
        ref.put(ID, id);
        assertEquals(0, getEntityCollection().find(ref).size());
    }

    public void assertCollectionItemsCount(DBObject ref, int count) {
        assertTrue(getEntityCollection().find(ref).size() == count);
    }

    public void assertCollectionCount(int count) {
        assertEquals(count, getEntityCollection().find().count());
    }

    public void assertObjectListFieldCount(String id, String listField, int count) {
        DBObject ref = new BasicDBObject();
        ref.put(ID, id);
        assertEquals(1, getEntityCollection().find(ref).size());
        DBObject obj = getEntityCollection().findOne(ref);
        assertTrue(obj.containsField(listField));
        assertEquals(count, ((List<DBObject>) obj.get(listField)).size());

    }

    public void assertObjectWithIdFieldValuePresent(Object id, String field, Object value) {
        DBObject ref = new BasicDBObject();
        ref.put(ID, id);
        ref.put(field, value);
        assertEquals(1, getEntityCollection().find(ref).count());
    }

    public void assertObjectWithIdFieldValueNotPresent(Object id, String field, Object value) {
        DBObject ref = new BasicDBObject();
        ref.put(ID, id);
        ref.put(field, value);
        assertEquals(0, getEntityCollection().find(ref).count());
    }

    public void insert(DBObject dbo) {
        getEntityCollection().insert(dbo);
    }

    public void insertJson(String... jsons) {
        for (String json : jsons) {
            getEntityCollection().save((DBObject) JSON.parse(json));
        }
    }

    public void dropDb() {
        m_imdb.getMongo().dropDatabase(m_imdb.getName());
    }

    public CoreContext getCoreContext() {
        return m_coreContext;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public MongoDbTemplate getImdb() {
        return m_imdb;
    }

    public void setImdb(MongoDbTemplate imdb) {
        m_imdb = imdb;
    }

    public PermissionManager getPermissionManager() {
        return m_permissionManager;
    }

    public void setPermissionManager(PermissionManager permissionManager) {
        m_permissionManager = permissionManager;
    }

    public DomainManager getDomainManager() {
        return m_domainManager;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }
}
