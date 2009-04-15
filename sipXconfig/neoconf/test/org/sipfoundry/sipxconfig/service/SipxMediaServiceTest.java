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

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.test.TestUtil;

import junit.framework.TestCase;

public class SipxMediaServiceTest extends TestCase {
    private SipxMediaService m_out = new SipxMediaService();
    
    public void setUp() {
        LocationsManager locationsManager = EasyMock.createMock(LocationsManager.class);
        locationsManager.getPrimaryLocation();
        EasyMock.expectLastCall().andReturn(TestUtil.createDefaultLocation());
        EasyMock.replay(locationsManager);
        m_out.setLocationsManager(locationsManager);
    }
    
    public void testGetVoicemailServer() {
        m_out.setVoicemailHttpsPort(9999);
        assertEquals("https://192.168.1.1:9999", m_out.getVoicemailServer());
    }
    
    public void testGetMediaServer() {
        assertEquals("192.168.1.1;transport=tcp", m_out.getMediaServer());
    }
}
