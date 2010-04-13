/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.device;

import junit.framework.TestCase;

public class DiscoveredDeviceTest extends TestCase {

    private DiscoveredDevice m_device1;
    private DiscoveredDevice m_device2;

    protected void setUp() throws Exception {
        m_device1 = new DiscoveredDevice();
        m_device2 = new DiscoveredDevice();
    }

    public void testEqual() {
        m_device1.setIpAddress("11.12.13.14");
        m_device1.setMacAddress("00:40:5A:17:C5:74");
        m_device1.setVendor("Cisco");

        m_device2.setIpAddress("11.12.13.14");
        m_device2.setMacAddress("00:40:5A:17:C5:74");
        m_device2.setVendor("Cisco");

        assertTrue(m_device1.equals(m_device2));
    }

    public void testNotEqual() {
        m_device1.setIpAddress("11.12.13.14");
        m_device1.setMacAddress("00:40:5A:17:C5:74");
        m_device1.setVendor("Cisco");

        m_device2.setIpAddress("11.12.13.14");
        m_device2.setMacAddress("FF:D3:CC:02:16:B1");
        m_device2.setVendor("Cisco");

        assertFalse(m_device1.equals(m_device2));
    }
}
