package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import org.sipfoundry.commons.mongo.MongoDbTemplate;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.permission.PermissionManager;

import com.mongodb.DBCollection;


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
