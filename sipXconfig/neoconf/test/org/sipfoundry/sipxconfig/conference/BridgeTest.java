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

import org.easymock.classextension.EasyMock;
import org.easymock.classextension.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.acd.BeanWithSettingsTestCase;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.setting.type.FileSetting;

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
        Bridge bridge = new Bridge();
        bridge.setModelFilesContext(TestHelper.getModelFilesContext());
        DeviceDefaults dd = new DeviceDefaults() ;
        dd.setDomainManager(TestHelper.getTestDomainManager("example.com")) ;
        bridge.setSystemDefaults(dd);
        assertTrue(bridge.getConferences().isEmpty());
        bridge.initialize() ;    
        
        assertEquals(bridge.getHost(), null);
        assertEquals(bridge.getPort(), 0);
        assertEquals(bridge.getSipDomain(), "example.com");
        assertEquals(bridge.getSipPort(), 15060);
        assertEquals(bridge.getDescription(), null);
        assertEquals(bridge.isEnabled(), false);
        assertEquals(bridge.getName(), null);
        assertEquals(bridge.getServiceUri(), "http://null:0/RPC2");
        
        bridge.setHost("bridge") ;
        bridge.setPort(8080) ;
        bridge.setDescription("Example Bridge");
        bridge.setEnabled(true);
        bridge.setName("foo");
        bridge.setAudioDirectory("/tmp") ;
        
        assertEquals(bridge.getHost(), "bridge");
        assertEquals(bridge.getPort(), 8080);
        assertEquals(bridge.getSipDomain(), "example.com");
        assertEquals(bridge.getSipPort(), 15060);
        assertEquals(bridge.getDescription(), "Example Bridge");
        assertEquals(bridge.isEnabled(), true);
        assertEquals(bridge.getName(), "foo");
        assertEquals(bridge.getServiceUri(), "http://bridge:8080/RPC2");
        assertEquals(bridge.getAudioDirectory(), "/tmp");
    }    
}
