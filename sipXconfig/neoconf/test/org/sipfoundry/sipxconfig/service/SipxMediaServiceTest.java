/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import static java.util.Collections.singleton;

import java.util.Arrays;

import org.easymock.EasyMock;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;

import junit.framework.TestCase;

import static org.easymock.EasyMock.expectLastCall;

public class SipxMediaServiceTest extends TestCase {
    private SipxMediaService m_out = new SipxMediaService();
    private LocationsManager m_locationsManager;
    
    public void setUp() {
        m_locationsManager = EasyMock.createMock(LocationsManager.class);
        Location location1 = new Location();
        location1.setName("location1");
        location1.setFqdn("location1");
        location1.setAddress("192.168.1.1");
        location1.setPrimary(true);

        Location location2 = new Location();
        location2.setName("location2");
        location2.setFqdn("location2");
        location2.setAddress("192.168.1.2");

        // Note: VoiceMail server and Media Server are not necessarily running on primary server,
        // and it depends on where the SipxMediaService is installed. In this test,
        // we "install" SipxMediaService on the non-primary location, location2.
        location2.setServices(singleton(new LocationSpecificService(m_out)));

        m_locationsManager.getLocationsForService(m_out);
        expectLastCall().andReturn(Arrays.asList(location2)).anyTimes();

        EasyMock.replay(m_locationsManager);
        m_out.setLocationsManager(m_locationsManager);
    }
    
    public void testGetVoicemailServer() {
        m_out.setVoicemailHttpsPort(9999);
        assertEquals("https://192.168.1.2:9999", m_out.getVoicemailServer());
    }
    
    public void testGetMediaServer() {
        assertEquals("192.168.1.2;transport=tcp", m_out.getMediaServer());
    }
}
