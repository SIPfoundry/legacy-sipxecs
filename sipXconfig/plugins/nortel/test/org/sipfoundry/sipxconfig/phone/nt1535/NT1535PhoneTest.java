/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.nt1535;

import java.io.InputStream;
import java.util.ArrayList;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phone.nt1535.NT1535Phone.DeviceConfigProfile;
import org.sipfoundry.sipxconfig.phone.nt1535.NT1535Phone.SystemConfigProfile;

public class NT1535PhoneTest extends TestCase {

    public void _testFactoryRegistered() {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
        assertNotNull(pc.newPhone(new PhoneModel("nt1535")));
    }

    public void testGetFileName() throws Exception {
        NT1535Phone phone = new NT1535Phone();
        phone.setSerialNumber("0011aabb4455");
        assertEquals("0011AABB4455.cfg", phone.getDeviceFileName());
    }

    public void testRestart() throws Exception {
        PhoneModel nortelModel = new PhoneModel("nt1535");
        Phone phone = new NT1535Phone();
        phone.setModel(nortelModel);

        PhoneTestDriver testDriver = PhoneTestDriver.supplyTestData(phone,true,false,false,true);
        phone.restart();

        testDriver.sipControl.verify();
    }

    public void testRestartNoLine() throws Exception {
        PhoneModel nortelModel = new PhoneModel("nt1535");
        Phone phone = new NT1535Phone();
        phone.setModel(nortelModel);

        PhoneTestDriver testDriver = PhoneTestDriver.supplyTestData(phone, new ArrayList<User>(), true);
        phone.restart();

        testDriver.sipControl.verify();
    }

    public void testGenerateSystemProfile() throws Exception {
        PhoneModel nortelModel = new PhoneModel("nt1535");
        NT1535Phone phone = new NT1535Phone();
        phone.setModel(nortelModel);

        PhoneTestDriver.supplyTestData(phone);
        phone.setProfileGenerator(TestHelper.getProfileGenerator());

        Profile profile = new SystemConfigProfile(phone, "profile");
        assertEquals("1.0/0.1.90S/profile", profile.getName());

        MemoryProfileLocation location = new MemoryProfileLocation();
        profile.generate(phone, location);

        InputStream expected = getClass().getResourceAsStream("expected_sysconf_2890d_sip.cfg");

        assertEquals(IOUtils.toString(expected), location.toString());
        expected.close();
    }

    public void testGenerateDeviceProfile() throws Exception {
        PhoneModel nt1535 = new PhoneModel("nt1535");
        NT1535Phone phone = new NT1535Phone();
        phone.setModel(nt1535);

        PhoneTestDriver.supplyTestData(phone);
        phone.setProfileGenerator(TestHelper.getProfileGenerator());

        Profile profile = new DeviceConfigProfile(phone, "profile");
        assertEquals("1.0/0.1.90S/profile", profile.getName());

        MemoryProfileLocation location = new MemoryProfileLocation();
        profile.generate(phone, location);

        InputStream expected = getClass().getResourceAsStream("expected_mac.cfg");

        assertEquals(IOUtils.toString(expected), location.toString());
        expected.close();
    }

}
