/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.device;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.EmergencyInfo;
import org.sipfoundry.sipxconfig.moh.MusicOnHoldManager;

import junit.framework.TestCase;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class DeviceDefaultsTest extends TestCase {

    public void testGetMusicOnHoldUri() {
        DeviceDefaults defaults = new DeviceDefaults();

        MusicOnHoldManager musicOnHoldManager = createMock(MusicOnHoldManager.class);
        musicOnHoldManager.getDefaultMohUri();
        expectLastCall().andReturn("sip:~~mh~@example.org").anyTimes();
        replay(musicOnHoldManager);

        defaults.setMusicOnHoldManager(musicOnHoldManager);

        assertEquals("sip:~~mh~@example.org", defaults.getMusicOnHoldUri());
    }

    public void testGetLikelyEmergencyInfo() {
        DeviceDefaults defaults = new DeviceDefaults();

        assertNull(defaults.getEmergencyAddress());
        assertNull(defaults.getEmergencyNumber());
        assertNull(defaults.getEmergencyPort());

        EmergencyInfo ei = new EmergencyInfo("gateway.example.com", 5060, "111");

        DialPlanContext dialPlanContext = createMock(DialPlanContext.class);
        dialPlanContext.getLikelyEmergencyInfo();
        expectLastCall().andReturn(ei).anyTimes();

        replay(dialPlanContext);

        defaults.setRouteEmergencyCallsDirectly(true);
        defaults.setDialPlanContext(dialPlanContext);

        assertEquals("gateway.example.com", defaults.getEmergencyAddress());
        assertEquals("111", defaults.getEmergencyNumber());
        assertEquals(5060, defaults.getEmergencyPort().intValue());

        defaults.setRouteEmergencyCallsDirectly(false);

        assertNull(defaults.getEmergencyAddress());
        assertNull(defaults.getEmergencyNumber());
        assertNull(defaults.getEmergencyPort());

        verify(dialPlanContext);
    }
}
