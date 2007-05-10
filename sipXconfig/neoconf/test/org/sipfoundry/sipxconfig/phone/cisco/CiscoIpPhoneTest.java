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
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class CiscoIpPhoneTest extends TestCase {

    private CiscoIpPhone m_phone;

    protected void setUp() {
        CiscoModel model = new CiscoModel();
        model.setBeanId(CiscoIpPhone.BEAN_ID);
        model.setMaxLineCount(6);
        model.setModelId("cisco7960");
        m_phone = new CiscoIpPhone();
        m_phone.setModel(model);
        PhoneTestDriver.supplyTestData(m_phone);
    }

    public void testGetSettings() {
        m_phone.getSettings();
    }

    public void testGenerate7960Profiles() throws Exception {
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(m_phone);
        m_phone.generateProfiles(location);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream(
                "expected-7960.cfg"));
        assertEquals(expected, location.toString());
    }
}
