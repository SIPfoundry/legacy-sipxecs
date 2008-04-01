package org.sipfoundry.sipxconfig.gateway;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;

public class SipTrunkTest extends TestCase {

    private static final int DEFAULT_PORT = 5060;
    
    public void testGetRouteWithoutPort() {
        SipTrunk out = new SipTrunk();
        SbcDevice sbcDevice = new SbcDevice();
        sbcDevice.setAddress("1.1.1.1");
        
        out.setSbcDevice(sbcDevice);
        
        assertEquals("1.1.1.1", out.getRoute());
    }
    
    public void testGetRouteWithPort() {
        SipTrunk out = new SipTrunk();
        SbcDevice sbcDevice = new SbcDevice();
        sbcDevice.setAddress("1.1.1.1");
        sbcDevice.setPort(5555);
        
        out.setSbcDevice(sbcDevice);
        
        assertEquals("1.1.1.1:5555", out.getRoute());
    }
    
    public void testGetRouteWithDefaultPort() {
        SipTrunk out = new SipTrunk();
        SbcDevice sbcDevice = new SbcDevice();
        sbcDevice.setAddress("1.1.1.1");
        sbcDevice.setPort(DEFAULT_PORT);
        
        out.setSbcDevice(sbcDevice);
        
        assertEquals("1.1.1.1", out.getRoute());
    }
}
