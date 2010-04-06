/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.gateway;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.gateway.Gateway.AddressTransport;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;

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

    public void testDefaults() {
        ModelFilesContext modelFilesContext = TestHelper.getModelFilesContext();
        SipTrunk sipTrunk = new SipTrunk();
        sipTrunk.setModelFilesContext(modelFilesContext);
        sipTrunk.initialize();

        sipTrunk.setAddressTransport(AddressTransport.NONE);
        assertEquals("UDP", sipTrunk.getSettingValue("itsp-account/itsp-transport"));
        sipTrunk.setAddressTransport(AddressTransport.TCP);
        assertEquals("TCP", sipTrunk.getSettingValue("itsp-account/itsp-transport"));
        sipTrunk.setAddressTransport(AddressTransport.UDP);
        assertEquals("UDP", sipTrunk.getSettingValue("itsp-account/itsp-transport"));
    }
}
