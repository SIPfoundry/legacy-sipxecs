/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.cisco;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.device.AbstractProfileGenerator;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class CiscoAtaPhoneTest extends TestCase {
    private CiscoModel m_model;

    protected void setUp() {
        m_model = new CiscoModel("ciscoAta");
    }

    public void testAtaProfile() throws Exception {
        m_model.setCfgPrefix("ata");
        m_model.setModelId("cisco18x");
        m_model.setMaxLineCount(2);
        String actual = runPhoneTest();

        String expected = IOUtils.toString(this.getClass().getResourceAsStream(
                "expected-ata18x.cfg"));
        assertNotNull(expected);
        assertEquals(expected, actual);
    }

    public void testNonAtaProfile() throws Exception {
        m_model.setCfgPrefix("gk");
        m_model.setModelId("cisco7912");
        m_model.setMaxLineCount(1);
        String actual = runPhoneTest();

        String expected = IOUtils.toString(this.getClass().getResourceAsStream(
                "expected-7912.cfg"));
        assertNotNull(expected);
        assertEquals(expected, actual);
    }

    String runPhoneTest() throws Exception {
        CiscoAtaPhone phone = new CiscoAtaPhone();

        phone.setTextFormatEnabled(true);
        phone.setModel(m_model);
        PhoneTestDriver.supplyTestData(phone);
        MemoryProfileLocation location = new MemoryProfileLocation();
        AbstractProfileGenerator pg = new CiscoAtaProfileWriter();
        phone.setProfileGenerator(pg);

        phone.generateProfiles(location);

        return location.toString();
    }
}
