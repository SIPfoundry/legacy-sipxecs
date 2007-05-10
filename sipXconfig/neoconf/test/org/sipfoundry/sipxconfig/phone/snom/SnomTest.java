package org.sipfoundry.sipxconfig.phone.snom;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class SnomTest extends TestCase {

    public void testGenerateProfiles() throws Exception {
        SnomPhone phone = new SnomPhone();
        SnomModel model = new SnomModel();
        model.setLabel("Snom 360");
        
        phone.setModel(model);
        PhoneTestDriver.supplyTestData(phone);
        
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);
        
        phone.generateProfiles(location);
        String expected = IOUtils.toString(this.getClass()
                .getResourceAsStream("expected-360.cfg"));

        System.err.println(location.toString());
        
        assertEquals(expected, location.toString());
    }

    public void testGetProfileName() {
        Phone phone = new SnomPhone();
        // it can be called without serial number
        assertEquals("snom.htm", phone.getPhoneFilename());
        phone.setSerialNumber("abc123");
        assertEquals("ABC123.htm", phone.getPhoneFilename());
    }
}
