/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.acd.BeanWithSettingsTestCase;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.service.LocationSpecificService;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;

public class BridgeTest extends BeanWithSettingsTestCase {

    public void testInsertConference() {
        Conference c = new Conference();
        c.setUniqueId();

        Bridge bridge = new Bridge();
        assertTrue(bridge.getConferences().isEmpty());
        bridge.addConference(c);

        assertEquals(1, bridge.getConferences().size());
        assertSame(c, bridge.getConferences().iterator().next());

        assertSame(bridge, c.getBridge());
    }

    public void testRemoveConference() {
        Conference c = new Conference();
        c.setUniqueId();

        Conference c1 = new Conference();
        c1.setUniqueId();

        Bridge bridge = new Bridge();
        assertTrue(bridge.getConferences().isEmpty());
        bridge.addConference(c);

        bridge.removeConference(c1);
        assertEquals(1, bridge.getConferences().size());

        bridge.removeConference(c);
        assertTrue(bridge.getConferences().isEmpty());
        assertNull(c.getBridge());
    }
    
    public void testAccessors()
    {
        Location location = new Location();
        SipxFreeswitchService sipxService = new SipxFreeswitchService();
        sipxService.setSettings(TestHelper.loadSettings("freeswitch/freeswitch.xml"));
        LocationSpecificService service = new LocationSpecificService(sipxService);
        service.setLocation(location);
        
        Bridge bridge = new Bridge();
        bridge.setService(service);
        
        bridge.setModelFilesContext(TestHelper.getModelFilesContext());
        DeviceDefaults dd = new DeviceDefaults() ;
        dd.setDomainManager(TestHelper.getTestDomainManager("example.com")) ;
        bridge.setSystemDefaults(dd);
        assertTrue(bridge.getConferences().isEmpty());
        
        // Test the defaults
        assertEquals(null, bridge.getHost());
        assertEquals(8080, bridge.getPort());
        assertEquals(15060, bridge.getSipPort());
        assertEquals(null, bridge.getDescription());
        assertEquals(false, bridge.isEnabled());
        assertEquals(null, bridge.getName());
        assertEquals("http://null:8080/RPC2", bridge.getServiceUri());
        
        // Test setting explicit values
        location.setFqdn("bridge");
        location.setName("Example Bridge");
        sipxService.setSettingValue(Bridge.FREESWITCH_XMLRPC_PORT, "8081");
        sipxService.setSettingValue(Bridge.FREESWITCH_SIP_PORT, "15061");
        bridge.setEnabled(true);
        bridge.setAudioDirectory("/tmp") ;
        
        assertEquals("bridge", bridge.getHost());
        assertEquals(8081, bridge.getPort());
        assertEquals(15061, bridge.getSipPort());
        assertEquals("Example Bridge", bridge.getDescription());
        assertEquals(true, bridge.isEnabled());
        assertEquals("http://bridge:8081/RPC2", bridge.getServiceUri());
        assertEquals("/tmp", bridge.getAudioDirectory());
    }    
}
