package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.domain.DomainManager;

import com.mongodb.DBCollection;

public class MongoTestCase extends TestCase {
    private static DBCollection m_collection;
    public final static String DBNAME = "imdb";
    public final static String COLL_NAME = "entity";
    public final static String DOMAIN = "mydomain.org";
    private CoreContext m_coreContext;
    private DomainManager m_dm;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        m_collection = MongoTestCaseHelper.initMongo(DBNAME, COLL_NAME);

        m_coreContext = createMock(CoreContext.class);
        m_coreContext.getDomainName();
        expectLastCall().andReturn(DOMAIN).anyTimes();
        m_coreContext.getAuthorizationRealm();
        expectLastCall().andReturn(DOMAIN).anyTimes();
        
        m_dm = createMock(DomainManager.class);
        m_dm.getDomainName();
        expectLastCall().andReturn(DOMAIN).anyTimes();
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
        MongoTestCaseHelper.destroyAllDbs();
    }

    public DomainManager getDomainManager() {
        return m_dm;
    }

}
