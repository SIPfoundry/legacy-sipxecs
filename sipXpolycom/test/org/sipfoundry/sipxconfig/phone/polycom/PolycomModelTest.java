package org.sipfoundry.sipxconfig.phone.polycom;

import junit.framework.TestCase;

public class PolycomModelTest extends TestCase {

    public void testGetVersion() {
        PolycomModel model = new PolycomModel();
        assertEquals(PolycomModel.VER_3_1_X, PolycomModel.getPhoneDeviceVersion("3.1.X"));
        assertEquals(PolycomModel.VER_3_2_X, PolycomModel.getPhoneDeviceVersion("3.2.X"));
        assertEquals(PolycomModel.VER_4_0_X, PolycomModel.getPhoneDeviceVersion("4.0.X"));
        assertEquals(PolycomModel.VER_2_0, PolycomModel.getPhoneDeviceVersion("X.X.X"));
    }
}
