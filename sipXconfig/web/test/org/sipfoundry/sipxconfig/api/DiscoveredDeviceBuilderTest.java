/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.api;

import org.sipfoundry.sipxconfig.device.DiscoveredDevice;

import junit.framework.TestCase;

public class DiscoveredDeviceBuilderTest extends TestCase {

    public void testToApi() {
        DiscoveredDeviceBuilder builder = new DiscoveredDeviceBuilder();
        DiscoveredDevice myDevice = new DiscoveredDevice();
        myDevice.setIpAddress("100.100.100.100");
        myDevice.setMacAddress("000011112222");
        myDevice.setVendor("testVendor");
        org.sipfoundry.sipxconfig.api.DiscoveredDevice apiDevice = new org.sipfoundry.sipxconfig.api.DiscoveredDevice();

        ApiBeanUtil.toApiObject(builder, apiDevice, myDevice);
        assertEquals(apiDevice.getIpAddress(),myDevice.getIpAddress());
        assertEquals(apiDevice.getMacAddress(),myDevice.getMacAddress());
        assertEquals(apiDevice.getVendor(),myDevice.getVendor());
    }

    public void testToMy() {
        DiscoveredDeviceBuilder builder = new DiscoveredDeviceBuilder();
        org.sipfoundry.sipxconfig.api.DiscoveredDevice apiDevice = new org.sipfoundry.sipxconfig.api.DiscoveredDevice();
        apiDevice.setIpAddress("100.100.100.100");
        apiDevice.setMacAddress("000011112222");
        apiDevice.setVendor("testVendor");
        DiscoveredDevice myDevice = new DiscoveredDevice();

        ApiBeanUtil.toMyObject(builder, myDevice, apiDevice);
        assertEquals(myDevice.getIpAddress(),apiDevice.getIpAddress());
        assertEquals(myDevice.getMacAddress(),apiDevice.getMacAddress());
        assertEquals(myDevice.getVendor(),apiDevice.getVendor());
    }
}
