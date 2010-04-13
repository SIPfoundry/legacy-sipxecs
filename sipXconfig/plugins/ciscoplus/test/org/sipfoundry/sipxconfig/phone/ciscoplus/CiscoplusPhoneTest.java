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

import java.util.Arrays;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
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
    }

    public void testGetSettings() {
        PhoneTestDriver.supplyTestData(m_phone);
        assertNotNull(m_phone.getSettings());
    }

    public void testGetFileName() {
        m_phone.setSerialNumber("0011aaBB4050");
        Profile[] profileTypes = m_phone.getProfileTypes();
        assertEquals(1, profileTypes.length);
        assertEquals("SEP0011AABB4050.cnf.xml", profileTypes[0].getName());
    }

    public void testGenerate7960Profiles() throws Exception {
        PhoneTestDriver.supplyTestData(m_phone);
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(m_phone);
        m_phone.generateProfiles(location);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("SEP111111111111.cnf.xml"));
        assertEquals(expected, location.toString());
    }

    public void testGenerate7960ProfilesTwoLine() throws Exception {
        User u1 = new User();
        u1.setUserName("200");
        u1.setSipPassword("200");
        u1.setLastName("200");

        User u2 = new User();
        u2.setUserName("201");
        u2.setSipPassword("201");
        u2.setLastName("201");

        // call this to inject dummy data
        PhoneTestDriver.supplyTestData(m_phone, Arrays.asList(new User[] {
            u1, u2
        }));
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(m_phone);
        m_phone.generateProfiles(location);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("SEP111111111111-2-lines.cnf.xml"));
        assertEquals(expected, location.toString());
    }
}
