/*
 *
 *
 * Copyright (c) 2004-2009 iscoord ltd.
 * Beustweg 12, 8032 Zurich, Switzerland
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxconfig.phone.isphone;



import java.util.Arrays;

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;


public class IsphoneTest extends TestCase {

    public void testGenerateProfiles() throws Exception {
        PhoneModel model = new PhoneModel("isphone");
        model.setModelId("isphone notes");
        model.setLabel("Isphone notes");
        model.setModelDir("isphone");
        model.setProfileTemplate("isphone/isphone.vm");

        IsphonePhone phone = preparePhone(model);

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        phone.generateProfiles(location);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("expected-isphone.xml"));

        assertEquals(expected.replace("\n", "").replace("\r", "").replace("\t", ""),
        				location.toString().replace("\n", "").replace("\r", "").replace("\t", ""));
    }

    private IsphonePhone preparePhone(PhoneModel model) {
        User user = new User();
        user.setUserName("mfast");
        user.setFirstName("Mike");
        user.setLastName("Fast");
        user.setSipPassword("secret");
        user.setIsShared(false);
        IsphonePhone phone = new IsphonePhone();
        phone.setSerialNumber("the isphone");

        phone.setModel(model);
        PhoneTestDriver.supplyTestData(phone, Arrays.asList(user));
        return phone;
    }

    public void testGetProfileName() {
        Phone phone = new IsphonePhone();
        phone.setSerialNumber("abc123");
        assertEquals("ABC123.xml", phone.getProfileFilename());
    }

}
