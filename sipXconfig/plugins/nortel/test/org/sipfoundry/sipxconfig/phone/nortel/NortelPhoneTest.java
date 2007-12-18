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
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phone.RestartException;

public class NortelPhoneTest extends TestCase {
    public void _testFactoryRegistered() {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
        assertNotNull(pc.newPhone(new PhoneModel("nortel")));
    }

    public void testGetFileName() throws Exception {
        NortelPhone phone = new NortelPhone();
        phone.setSerialNumber("0011aabb4455");
        assertEquals("0011AABB4455.cfg", phone.getProfileFilename());
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

    public void testGenerateNortel1140() throws Exception {
        PhoneModel nortelModel = new PhoneModel("nortel");
        nortelModel.setProfileTemplate("nortel/mac.cfg.vm");
        nortelModel.setModelId("nortel1140");
        NortelPhone phone = new NortelPhone();
        phone.setModel(nortelModel);
        PhoneTestDriver.supplyTestData(phone);

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        phone.generateProfiles(location);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("mac_1140.cfg"));
        assertEquals(expected, location.toString());
    }
}
