package org.sipfoundry.sipxconfig.commserver;

import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;


public class LocationsManagerImplTestDb extends IntegrationTestCase {
    private LocationsManager m_locationsManager;
    
    @Override
    public void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    public void testDeletePrimaryLocation() throws Exception {
        sql("commserver/SeedLocations.sql");
        Location location = m_locationsManager.getPrimaryLocation();
        try {
            m_locationsManager.deleteLocation(location);
            fail("Deletion of primary location failed");
        } catch (UserException e) {
        }
    }

    public void testDelete() throws Exception {
        Location location = new Location();
        location.setName("test location");
        location.setAddress("10.1.1.1");
        location.setFqdn("localhost");
        location.setRegistered(true);
        location.setPrimary(true);
        m_locationsManager.saveLocation(location);

        Location location2 = new Location();
        location2.setName("test location2");
        location2.setAddress("10.1.1.2");
        location2.setFqdn("localhost1");
        location2.setRegistered(false);
        m_locationsManager.saveLocation(location2);

        Location[] locationsBeforeDelete = m_locationsManager.getLocations();
        assertEquals(2, locationsBeforeDelete.length);

        Location locationToDelete = m_locationsManager.getLocationByAddress("10.1.1.2");
        m_locationsManager.deleteLocation(locationToDelete);

        Location[] locationsAfterDelete = m_locationsManager.getLocations();
        assertEquals(1, locationsAfterDelete.length);
        assertEquals("localhost", locationsAfterDelete[0].getFqdn());
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}
