package org.sipfoundry.sipxconfig.commserver;

import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertCollectionCount;

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.test.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.context.ApplicationContext;
import org.springframework.data.mongodb.core.MongoTemplate;

import com.mongodb.DBCollection;


public class LocationsManagerImplTestDb extends SipxDatabaseTestCase {
    private LocationsManager m_out;
    
    
    public void setUp() {
        ApplicationContext app = TestHelper.getApplicationContext();
        m_out = (LocationsManager) app.getBean(LocationsManager.CONTEXT_BEAN_NAME);
    }

    public void testDeletePrimaryLocation() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsert("commserver/seedLocations.xml");
        Location location = m_out.getPrimaryLocation();
        try {
            m_out.deleteLocation(location);
            fail("Deletion of primary location failed");
        } catch (UserException e) {

        }
    }

    public void testDelete() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsert("commserver/clearLocations.xml");
        Location location = new Location();
        location.setName("test location");
        location.setAddress("10.1.1.1");
        location.setFqdn("localhost");
        location.setRegistered(true);
        location.setPrimary(true);
        m_out.saveLocation(location);

        Location location2 = new Location();
        location2.setName("test location2");
        location2.setAddress("10.1.1.2");
        location2.setFqdn("localhost1");
        location2.setRegistered(false);
        m_out.saveLocation(location2);

        Location[] locationsBeforeDelete = m_out.getLocations();
        assertEquals(2, locationsBeforeDelete.length);

        Location locationToDelete = m_out.getLocationByAddress("10.1.1.2");
        m_out.deleteLocation(locationToDelete);

        Location[] locationsAfterDelete = m_out.getLocations();
        assertEquals(1, locationsAfterDelete.length);
        assertEquals("localhost", locationsAfterDelete[0].getFqdn());
    }
}
