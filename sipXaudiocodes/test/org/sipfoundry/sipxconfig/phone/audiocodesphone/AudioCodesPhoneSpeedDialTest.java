package org.sipfoundry.sipxconfig.phone.audiocodesphone;

import java.io.InputStream;
import java.util.Arrays;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;
import org.sipfoundry.sipxconfig.test.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class AudioCodesPhoneSpeedDialTest extends TestCase {
    private ProfileGenerator m_pg;
    private MemoryProfileLocation m_location;

    protected void setUp() {
        m_location = new MemoryProfileLocation();
        VelocityProfileGenerator pg = new VelocityProfileGenerator();
        pg.setVelocityEngine(TestHelper.getVelocityEngine());
        m_pg = pg;
    }

    public void testGenerateSpeedDials() throws Exception {
        Button button1 = new Button();
        button1.setNumber("200");
        Button button2 = new Button();
        button2.setNumber("201");
        button2.setBlf(true);

        SpeedDial speedDial = new SpeedDial();
        speedDial.setButtons(Arrays.asList(button1, button2));

        AudioCodesPhoneSpeedDial dir = new AudioCodesPhoneSpeedDial(speedDial);

        m_pg.generate(m_location, dir, null, "profile");

        InputStream expectedProfile = getClass().getResourceAsStream("expected-speeddial.txt");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        assertEquals(expected, m_location.toString());
    }

}
