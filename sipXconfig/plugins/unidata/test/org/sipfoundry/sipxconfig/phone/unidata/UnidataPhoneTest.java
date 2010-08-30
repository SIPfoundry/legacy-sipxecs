/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.unidata;

import java.io.InputStream;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class UnidataPhoneTest extends TestCase {
    public void testGetFileName() throws Exception {
    	UnidataPhone phone = new UnidataPhone();
        phone.setSerialNumber("001122334455");
        assertEquals("e1_001122334455.ini", phone.getProfileFilename());
    }

    public void testGenerateTypicalProfile() throws Exception {
    	UnidataPhone phone = new UnidataPhone();
        PhoneModel model = new PhoneModel("unidata");
        model.setModelId("unidatawpu7700");
        phone.setModel(model);

        // call this to inject dummy data
        PhoneTestDriver.supplyTestData(phone);
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        ProfileContext context = new ProfileContext(phone, "unidata/config.vm");
        phone.getProfileGenerator().generate(location, context, null, "ignore");
        
        InputStream expectedProfile = getClass().getResourceAsStream("test-e1_MAC.ini");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        assertEquals(expected, location.toString());
    }
}
