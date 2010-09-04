/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.nortel;

import java.util.ArrayList;


import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.device.RestartException;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phone.nortel.NortelPhone;
import org.sipfoundry.sipxconfig.phone.nortel.NortelPhone.FeatureKeyListProfile;
import org.sipfoundry.sipxconfig.phone.nortel.NortelPhone.PhonebookProfile;

/**
 * Tests the phone profile
 * Tests the featurekeylist profile reference inside the phone profile
 * Tests the phonebook profile reference inside the phone profile
 */
public class NortelPhoneTest extends TestCase {

    public void _testFactoryRegistered() {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
        assertNotNull(pc.newPhone(new PhoneModel("nortel")));
    }

    public void testGetFileName() throws Exception {
        NortelPhone phone = new NortelPhone();
        phone.setSerialNumber("0011aabb4455");
        assertEquals("SIP0011AABB4455.cfg", phone.getProfileFilename());
    }

    public void testRestart() throws Exception {
        PhoneModel nortelModel = new PhoneModel("nortel");
        Phone phone = new NortelPhone();
        phone.setModel(nortelModel);

        PhoneTestDriver testDriver = PhoneTestDriver.supplyTestData(phone);
        phone.restart();

        testDriver.sipControl.verify();
    }

    public void testRestartNoLine() throws Exception {
        PhoneModel nortelModel = new PhoneModel("nortel");
        Phone phone = new NortelPhone();
        phone.setModel(nortelModel);

        PhoneTestDriver.supplyTestData(phone, new ArrayList<User>());
        try {
            phone.restart();
            fail();
        } catch (RestartException re) {
            assertTrue(true);
        }
    }

   /**
     * This test will verify that the Phone Mac file is the same as the mac_1140.cfg
     * It also verifies that the filenames for featurekeylist and phonebook are generated correctly
     * witin the mac file
     */
    public void testGenerateNortel1140() throws Exception {
        Phone phone = createPhone();
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);
        phone.generateProfiles(location);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("mac_1140.cfg"));
        String actual = location.toString("SIP001122334455.cfg");
        System.out.println("*** BEGIN actual profile content. ***");
        System.out.println(actual);
        System.out.println("*** END actual profile content. ***");
        assertEquals(expected, actual);
    }

    public void testGenerateNortel1140_32() throws Exception {
        NortelPhone phone = (NortelPhone)createPhone();
        phone.setDeviceVersion(NortelPhoneModel.FIRM_3_2);
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);
        phone.generateProfiles(location);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("mac_1140_3.2.cfg"));
        String actual = location.toString("SIP001122334455.cfg");
        assertEquals(expected, actual);
        System.out.println("*** BEGIN actual profile content. ***");
        System.out.println(actual);
        System.out.println("*** END actual profile content. ***");
    }

    private Phone createPhone()
    {
        PhoneModel nortelModel = new PhoneModel("nortel");
        nortelModel.setProfileTemplate("nortel/mac.cfg.vm");
        nortelModel.setModelId("nortel1140");
        NortelPhone phone = new NortelPhone();
        phone.setModel(nortelModel);
        PhoneTestDriver.supplyTestData(phone);
        phone.setSerialNumber("001122334455");
        return phone;
    }


    /**
     * Tests that the phonebook profile is used when phonebook management is enabled.
     */
    public void testPhonebookManagementEnabled() throws Exception {
        PhoneModel nortelModel = new PhoneModel("nortel");
        Phone phone = new NortelPhone();
        phone.setModel(nortelModel);
        PhoneTestDriver.supplyTestData(phone, true);

        // Should return three profiles - the regular profile and the phonebook
        // profile and the featurekeyList profile.
        Profile[] profileTypes = phone.getProfileTypes();
        assertEquals(3, profileTypes.length);
        assertTrue(profileTypes[0].getClass().equals(Profile.class));
        assertTrue(profileTypes[1].getClass().equals(PhonebookProfile.class));
        assertTrue(profileTypes[2].getClass().equals(FeatureKeyListProfile.class));

    }

    /**
     * Tests that the featureKeyList profile is used when featureKeyList management is enabled
     */
    public void testFeatureKeyListManagementEnabled() throws Exception {
        PhoneModel nortelModel = new PhoneModel("nortel");
        Phone phone = new NortelPhone();
        phone.setModel(nortelModel);
        //enables phonebook and speedial profiles(true,true,,) for phone
        PhoneTestDriver.supplyTestData(phone, true,true,false,false);

        // Should return three profiles - the regular profile and the phonebook
        // profile and the featurekeyList profile.
        Profile[] profileTypes = phone.getProfileTypes();
        assertEquals(3, profileTypes.length);
        assertTrue(profileTypes[0].getClass().equals(Profile.class));
        assertTrue(profileTypes[1].getClass().equals(PhonebookProfile.class));
        assertTrue(profileTypes[2].getClass().equals(FeatureKeyListProfile.class));

    }


    /**
     * Tests that the phonebook profile is not used when phonebook management is disabled.
     */
    public void testPhonebookManagementDisabled() throws Exception {
        PhoneModel nortelModel = new PhoneModel("nortel");
        Phone phone = new NortelPhone();
        phone.setModel(nortelModel);
        PhoneTestDriver.supplyTestData(phone, false);

        // Should only return one Profile.
        Profile[] profileTypes = phone.getProfileTypes();
        assertEquals(1, profileTypes.length);
        // Make sure it's not a PhonebookProfile. We can't use instanceof to
        // check the type, because since a PhonebookProfile is a Profile, the
        // result would be true. So we have to compare the classes directly.
        assertTrue(profileTypes[0].getClass().equals(Profile.class));
    }

}
