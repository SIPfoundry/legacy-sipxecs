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
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class CiscoplusPhoneTest extends TestCase {

    private CiscoplusPhone m_phone;

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

    public void testGenerate7960Profiles() throws Exception {
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(m_phone);
        m_phone.generateProfiles(location);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("SEP111111111111.cnf.xml"));
        assertEquals(expected, location.toString());
    }
}
