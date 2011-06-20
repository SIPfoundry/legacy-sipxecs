package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import static org.sipfoundry.commons.mongo.MongoConstants.ID;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_ENABLED;
import static org.sipfoundry.commons.mongo.MongoConstants.PERMISSIONS;
import static org.sipfoundry.commons.mongo.MongoConstants.UID;

import java.io.InputStream;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.speeddial.ResourceLists;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;

public class SpeedDialsTestIntegration extends IntegrationTestCase {

    private SpeedDials m_speedDials;
    private CoreContext m_coreContext;
    private ResourceLists m_resourceLists;

    public void testGenerateResourceLists() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        loadDataSet("admin/commserver/imdb/speeddials.db.xml");
        MongoTestCaseHelper.initMongo("imdb", "entity");
        MongoTestCaseHelper.dropDb("imdb");
        DBCollection collection = MongoTestCaseHelper.initMongo("imdb", "entity");
        m_speedDials.setDbCollection(collection);

        User userA = m_coreContext.loadUserByUserName("user_a");
        User userC = m_coreContext.loadUserByUserName("user_c");
        User userB = m_coreContext.loadUserByUserName("user_name_0");
        User userD = m_coreContext.loadUserByUserName("user_name_1");

        List<String> prmlist = Arrays.asList(PermissionName.SUBSCRIBE_TO_PRESENCE.getName());
        List<String> prmlistNoSubscribe = Arrays.asList(PermissionName.MOBILE.getName());

        DBObject user1 = new BasicDBObject().append(ID, "User9991").append(UID, "user_a").append(IM_ENABLED, false);
        user1.put(PERMISSIONS, prmlist);
        DBObject user2 = new BasicDBObject().append(ID, "User9992").append(UID, "user_c").append(IM_ENABLED, false);
        user2.put(PERMISSIONS, prmlist);
        DBObject user3 = new BasicDBObject().append(ID, "User9993").append(UID, "user_name_0")
                .append(IM_ENABLED, true);
        user3.put(PERMISSIONS, prmlistNoSubscribe);
        DBObject user4 = new BasicDBObject().append(ID, "User9994").append(UID, "user_name_1")
                .append(IM_ENABLED, true);
        user4.put(PERMISSIONS, prmlistNoSubscribe);

        m_speedDials.generate(userA, user1);
        m_speedDials.generate(userC, user2);
        m_speedDials.generate(userB, user3);
        m_speedDials.generate(userD, user4);

        String generated = AbstractConfigurationFile.getFileContent(m_resourceLists, null);
        InputStream referenceXmlStream = getClass().getResourceAsStream("resource-lists.test.xml");
        assertEquals(IOUtils.toString(referenceXmlStream), generated);
        MongoTestCaseHelper.dropDb("imdb");
    }

    public void setSpeeddialDataSet(SpeedDials speedDials) {
        m_speedDials = speedDials;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setResourceListGenerator(ResourceLists resourceLists) {
        m_resourceLists = resourceLists;
    }
}
