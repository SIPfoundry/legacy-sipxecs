/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.linksys;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class LinksysPhoneTest extends TestCase {

    private Linksys m_phone;

    @Override
    protected void setUp() {
        LinksysModel model = new LinksysModel("linksysPhone");
        model.setMaxLineCount(3);
        model.setModelId("linksys942");
        model.setProfileTemplate("linksys/config.vm");
        model.setModelDir("linksys");
        model.setPsn("942");
        m_phone = new LinksysPhone();
        m_phone.setModel(model);
        PhoneTestDriver.supplyTestData(m_phone);
    }

    public void testGetSettings() {
        assertNotNull(m_phone.getSettings());
    }

    public void testGenerate7960Profiles() throws Exception {
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(m_phone);
        m_phone.generateFiles(location);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("spa942.cfg"));
        assertEquals(expected, location.toString());
    }

    public void testCopyProfile() {
        ProfileGenerator pg = createMock(ProfileGenerator.class);
        pg.copy(null, "linksys/default.cfg", "spa942.cfg");
        replay(pg);

        m_phone.setProfileGenerator(pg);
        m_phone.copyFiles(null);

        verify(pg);
    }

    public void testGetProfileFilename() {
        m_phone.setSerialNumber("1a2b33445566");
        assertEquals("spa1A2B33445566.cfg", m_phone.getProfileFilename());
    }
}
