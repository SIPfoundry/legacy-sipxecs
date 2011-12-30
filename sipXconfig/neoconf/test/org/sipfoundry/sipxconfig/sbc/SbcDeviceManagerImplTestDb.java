package org.sipfoundry.sipxconfig.sbc;

import org.sipfoundry.sipxconfig.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class SbcDeviceManagerImplTestDb extends IntegrationTestCase {
    private SbcDeviceManager m_sbcDeviceManager;
    private LocationsManager m_locationsManager;    
    private Location m_location;
    
    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        db().execute("insert into location (location_id, name, fqdn, ip_address) values (1, 'x', 'y', '10.1.2.5')");
        m_location = m_locationsManager.getLocation(1);
    }
    
    public void testGetSbcBridge() throws Exception {
        BridgeSbc bridgeSbc = m_sbcDeviceManager.newBridgeSbc(m_location);
        assertEquals("sipXbridge-" + m_location.getId().toString(), bridgeSbc.getName());
        assertEquals("10.1.2.5", bridgeSbc.getAddress());
        assertEquals("Internal SBC on y", bridgeSbc.getDescription());
    }
    
    public void testAutoRemove() throws Exception {
        m_locationsManager.deleteLocation(m_location);
        BridgeSbc bridgeSbc = m_sbcDeviceManager.getBridgeSbc(m_location);
        assertTrue(bridgeSbc == null);
    }

    public void setSbcDeviceManager(SbcDeviceManager sbcDeviceManager) {
        m_sbcDeviceManager = sbcDeviceManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}
