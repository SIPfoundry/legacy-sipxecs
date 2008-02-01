/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.ipdialog;

import java.io.InputStream;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class IpDialogPhoneTest extends TestCase {

    public void testGetFileName() throws Exception {
        IpDialogPhone phone = new IpDialogPhone();
        phone.setSerialNumber("123456789123");
        assertEquals("SipTone/config/ipdSIP123456789123.xml", phone.getProfileFilename());
    }

    public void testGenerateTypicalProfile() throws Exception {
        IpDialogPhone phone = new IpDialogPhone();
        PhoneModel model = new PhoneModel("ipDialog");
        model.setProfileTemplate("ipDialog/ipDialog.vm");
        phone.setModel(model);
        // call this to inject dummy data
        PhoneTestDriver.supplyTestData(phone);
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);
        phone.generateProfiles(location);
        InputStream expectedProfile = getClass().getResourceAsStream("expected-config");
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();
        assertEquals(expected, location.toString());
    }
}
