/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.counterpath;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class CounterpathPhoneTest extends TestCase {

    public void testGetFileName() throws Exception {
        CounterpathPhone phone = new CounterpathPhone();
        phone.setSerialNumber("0011AABB4455");
        assertEquals("0011AABB4455.ini", phone.getProfileFilename());
    }

    public void testGenerateCounterpathCMCEnterprise() throws Exception {
        CounterpathPhoneModel counterpathModel = new CounterpathPhoneModel("counterpath");
        counterpathModel.setProfileTemplate("counterpath/counterpath.ini.vm");
        counterpathModel.setModelId("counterpathCMCEnterprise");
        CounterpathPhone phone = new CounterpathPhone();
        phone.setModel(counterpathModel);
        phone.setDefaults(new DeviceDefaults());
        PhoneTestDriver.supplyTestData(phone,true,true);

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        phone.generateProfiles(location);

        String expected = IOUtils.toString(getClass().getResourceAsStream("cmc-enterprise.ini"));
        assertEquals(expected, location.toString());
    }
}