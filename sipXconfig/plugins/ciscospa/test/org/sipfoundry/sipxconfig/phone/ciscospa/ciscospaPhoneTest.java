package org.sipfoundry.sipxconfig.phone.ciscospa;

import java.io.InputStream;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class ciscospaPhoneTest extends TestCase {

    public void testGenerateTypicalProfile() throws Exception {
        ciscospaPhone phone = new ciscospaPhone();
        PhoneModel model = new PhoneModel("ciscospaPhone");
        model.setProfileTemplate("ciscospa/config.vm");
        phone.setModel(model);
        // call this to inject dummy data
        PhoneTestDriver.supplyTestData(phone);
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        phone.generateProfiles(location);

        InputStream expectedProfile = getClass().getResourceAsStream("spa508g.cfg");
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        assertEquals(expected, location.toString());
    }
}
