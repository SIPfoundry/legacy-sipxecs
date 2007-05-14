package org.sipfoundry.sipxconfig.phone.snom;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class SnomTest extends TestCase {

    public void testGenerateProfiles() throws Exception {
        SnomPhone phone = new SnomPhone();
        PhoneModel model = new PhoneModel("snom");
        model.setLabel("Snom 360");
        model.setModelDir("snom");
        model.setProfileTemplate("snom/snom.vm");
        
        phone.setModel(model);
        PhoneTestDriver.supplyTestData(phone);
        
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);
        
        phone.generateProfiles(location);
        String expected = IOUtils.toString(this.getClass()
                .getResourceAsStream("expected-360.cfg"));

        assertEquals(expected, location.toString());
    }

    public void testGetProfileName() {
        Phone phone = new SnomPhone();
        // it can be called without serial number
        assertEquals("snom.htm", phone.getProfileFilename());
        phone.setSerialNumber("abc123");
        assertEquals("ABC123.htm", phone.getProfileFilename());
    }
}
