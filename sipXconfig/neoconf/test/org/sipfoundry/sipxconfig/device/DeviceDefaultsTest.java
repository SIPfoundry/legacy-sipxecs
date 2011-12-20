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

import junit.framework.TestCase;

import org.easymock.classextension.EasyMock;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dialplan.EmergencyInfo;
import org.sipfoundry.sipxconfig.moh.MohAddressFactory;

public class DeviceDefaultsTest extends TestCase {

    public void testGetMusicOnHoldUri() {        
        DeviceDefaults defaults = new DeviceDefaults();
        MohAddressFactory moh = EasyMock.createMock(MohAddressFactory.class);
        moh.getDefaultMohUri();
        EasyMock.expectLastCall().andReturn("sip:~~mh~@example.org").anyTimes();
        EasyMock.replay(moh);

        defaults.setMohAddressFactory(moh);

        assertEquals("sip:~~mh~@example.org", defaults.getMusicOnHoldUri());
    }

    public void testGetLikelyEmergencyInfo() {
        DeviceDefaults defaults = new DeviceDefaults();

        assertNull(defaults.getEmergencyAddress());
        assertNull(defaults.getEmergencyNumber());
        assertNull(defaults.getEmergencyPort());

        EmergencyInfo ei = new EmergencyInfo("gateway.example.com", 5060, "111");

        DialPlanContext dialPlanContext = EasyMock.createMock(DialPlanContext.class);
        dialPlanContext.getLikelyEmergencyInfo();
        EasyMock.expectLastCall().andReturn(ei).anyTimes();

        EasyMock.replay(dialPlanContext);

        defaults.setRouteEmergencyCallsDirectly(true);
        defaults.setDialPlanContext(dialPlanContext);

        assertEquals("gateway.example.com", defaults.getEmergencyAddress());
        assertEquals("111", defaults.getEmergencyNumber());
        assertEquals(5060, defaults.getEmergencyPort().intValue());

        defaults.setRouteEmergencyCallsDirectly(false);

        assertNull(defaults.getEmergencyAddress());
        assertNull(defaults.getEmergencyNumber());
        assertNull(defaults.getEmergencyPort());

        EasyMock.verify(dialPlanContext);
    }
}
