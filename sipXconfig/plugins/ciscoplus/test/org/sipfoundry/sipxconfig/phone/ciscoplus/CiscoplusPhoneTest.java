/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.ciscoplus;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class CiscoplusPhoneTest extends TestCase {

    private CiscoplusPhone m_phone;

    @Override
    protected void setUp() {
        CiscoplusModel model = new CiscoplusModel("ciscoplus");
        model.setMaxLineCount(3);
        model.setModelId("ciscoplus7961G");
        model.setProfileTemplate("ciscoplus/config.vm");
        m_phone = new CiscoplusPhone();
        m_phone.setModel(model);
        PhoneTestDriver.supplyTestData(m_phone);
    }

    public void testGetSettings() {
        assertNotNull(m_phone.getSettings());
    }

    public void testGetFileName() {
        m_phone.setSerialNumber("0011aaBB4050");
        Profile[] profileTypes = m_phone.getProfileTypes();
        assertEquals(1, profileTypes.length);
        assertEquals("SEP0011AABB4050.cnf.xml", profileTypes[0].getName());
    }

    public void testGenerate7960Profiles() throws Exception {
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(m_phone);
        m_phone.generateProfiles(location);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("SEP111111111111.cnf.xml"));
        assertEquals(expected, location.toString());
    }
}
