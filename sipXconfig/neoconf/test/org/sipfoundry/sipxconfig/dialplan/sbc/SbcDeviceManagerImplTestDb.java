package org.sipfoundry.sipxconfig.admin.dialplan.sbc;

import static java.util.Arrays.asList;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.test.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.context.ApplicationContext;

public class SbcDeviceManagerImplTestDb extends SipxDatabaseTestCase {
    private SbcDeviceManager m_sdm;
    private LocationsManager m_locationsManager;    
    
    public void setUp() {
        ApplicationContext app = TestHelper.getApplicationContext();
        m_locationsManager = (LocationsManager) app.getBean(LocationsManager.CONTEXT_BEAN_NAME);
        m_sdm = (SbcDeviceManager) app.getBean(SbcDeviceManager.CONTEXT_BEAN_NAME);
    }
    
    public void testGetSbcBridge() {
        Location location = new Location();
        location.setName("test location");
        location.setAddress("10.1.2.6");
        location.setFqdn("location1");
        location.setInstalledBundles(asList("borderControllerBundle"));
        m_locationsManager.saveLocation(location);

        BridgeSbc bridgeSbc = m_sdm.getBridgeSbc(location);
        assertEquals("sipXbridge-" + location.getId().toString(), bridgeSbc.getName());
        assertEquals("10.1.2.6", bridgeSbc.getAddress());
        assertEquals("Internal SBC on location1", bridgeSbc.getDescription());

        m_locationsManager.deleteLocation(location);
        bridgeSbc = m_sdm.getBridgeSbc(location);
        assertTrue(bridgeSbc==null);
    }

}
