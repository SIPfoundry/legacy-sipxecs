/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.clearone;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.AbstractProfileGenerator;
import org.sipfoundry.sipxconfig.device.FileSystemProfileLocation;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class ClearonePhoneTest extends TestCase {
    public void _testFactoryRegistered() {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
        assertNotNull(pc.newPhone(new PhoneModel("clearone")));
    }

    public void testGetFileName() throws Exception {
        ClearonePhone phone = new ClearonePhone();
        phone.setSerialNumber("0011aabb4455");
        assertEquals("C1MAXIP_0011AABB4455.txt", phone.getProfileFilename());
    }

    public void testGenerateTypicalProfile() throws Exception {
        ClearonePhone phone = new ClearonePhone();
        PhoneModel model = new PhoneModel("clearone");
        model.setProfileTemplate("clearone/C1MAXIP.txt.vm");
        phone.setModel(model);

        // call this to inject dummy data
        PhoneTestDriver.supplyTestData(phone);
        String parentDir = TestHelper.getTestDirectory() + "/clearone";
        FileSystemProfileLocation location = new FileSystemProfileLocation();
        location.setParentDir(parentDir);
        AbstractProfileGenerator profileGenerator = TestHelper.getProfileGenerator();
        phone.setProfileGenerator(profileGenerator);

        phone.generateProfiles(location);

        String expected = getResourceAsString("C1MAXIP.txt");

        File actualProfileFile = new File(parentDir, phone.getProfileFilename());
        assertTrue(actualProfileFile.exists());
        String actual = IOUtils.toString(new FileReader(actualProfileFile));

        compareProfiles(expected, actual);

        String expectedDialPlan = getResourceAsString("c1dialplan.txt");
        File actualDialPlanFile = new File(parentDir, phone.getDialplanFileName());
        assertTrue(actualDialPlanFile.exists());
        String actualDialPlan = IOUtils.toString(new FileReader(actualDialPlanFile));

        compareProfiles(expectedDialPlan, actualDialPlan);

        phone.removeProfiles(location);

        assertFalse(actualProfileFile.exists());
        assertFalse(actualDialPlanFile.exists());
    }

    public String getResourceAsString(String name) throws IOException {
        InputStream stream = getClass().getResourceAsStream(name);
        assertNotNull(name + " does not exist", stream);
        String result = IOUtils.toString(stream);
        stream.close();
        return result;
    }

    public void compareProfiles(String expected, String actual) {
        String expectedLines[] = StringUtils.split(expected.replaceAll(" +", " "), "\n");
        String actualLines[] = StringUtils.split(actual.replaceAll(" +", " "), "\n");

        assertEquals(expectedLines.length, actualLines.length);
        for (int i = 0; i < actualLines.length; i++) {
            assertEquals(expectedLines[i], actualLines[i]);
        }
    }
}
