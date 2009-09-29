/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.kphone;

import java.io.InputStream;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class KPhoneTest extends TestCase {
    private KPhone m_phone;

    protected void setUp() throws Exception {
        m_phone = new KPhone();
        PhoneModel phoneModel = new PhoneModel("kphone");
        phoneModel.setProfileTemplate("kphone/kphonerc.vm");
        m_phone.setModel(phoneModel);

    }

    public void testGenerateTypicalProfile() throws Exception {
        // call this to inject dummy data
        PhoneTestDriver.supplyTestData(m_phone);
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(m_phone);

        m_phone.generateProfiles(location);
        InputStream expectedProfile = getClass().getResourceAsStream("default-kphonerc");
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        // Display name because value comes from LineSettings now, not User object
        // kphone does not store Display name directory, but uses it as part of URI
        // would need a URI parser to get it back.
        assertEquals(expected, location.toString());
    }

    public void testGenerateEmptyProfile() throws Exception {
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(m_phone);
        m_phone.setModelFilesContext(TestHelper.getModelFilesContext());

        // All phones in system have a unique id, this will be important for
        // selecting which profile to download
        m_phone.setSerialNumber("000000000000");

        // method to test
        m_phone.generateProfiles(location);

        // test output file is a copy of the basic template and located in same directory
        // as this java source file
        InputStream expectedProfile = getClass().getResourceAsStream("empty-kphonerc");
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        assertEquals(expected, location.toString());
    }
}
