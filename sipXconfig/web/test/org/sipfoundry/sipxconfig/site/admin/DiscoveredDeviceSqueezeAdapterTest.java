/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import org.sipfoundry.sipxconfig.device.DiscoveredDevice;

import junit.framework.TestCase;

public class DiscoveredDeviceSqueezeAdapterTest extends TestCase {

    private DiscoveredDeviceSqueezeAdapter adapter;

    protected void setUp() throws Exception {
        adapter = new DiscoveredDeviceSqueezeAdapter();
    }

    public void testGetPrimaryKey() {
        DiscoveredDevice device = new DiscoveredDevice();
        device.setMacAddress("00:40:5A:17:C5:74");
        assertEquals("00:40:5A:17:C5:74",adapter.getPrimaryKey(device));
    }

    public void testGetValue() {
        Object deviceObject = adapter.getValue("00:40:5A:17:C5:74");
        DiscoveredDevice device = (DiscoveredDevice) deviceObject;
        assertEquals("00:40:5A:17:C5:74", device.getMacAddress());
    }
}
