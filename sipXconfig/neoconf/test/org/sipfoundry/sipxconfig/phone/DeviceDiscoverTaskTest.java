/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import java.io.File;
import java.util.List;

import org.apache.commons.io.FileUtils;
import org.dom4j.Document;
import org.dom4j.DocumentHelper;
import org.sipfoundry.sipxconfig.device.DiscoveredDevice;
import org.sipfoundry.sipxconfig.test.TestUtil;

import junit.framework.TestCase;

public class DeviceDiscoverTaskTest extends TestCase {

    private DeviceFinder m_deviceFinder;

    public void setUp() {
        m_deviceFinder = new DeviceFinder();
    }

    public void testRunWithFailedDiscover() {
        DeviceDiscoverTask out = new DeviceDiscoverTask(m_deviceFinder) {
            Document getDiscoveredDevicesXmlDocument() {
                return null;
            }
        };

        out.run();
        assertEquals(DeviceFinder.FAILED, m_deviceFinder.getState());
    }

    public void testRunWithSuccessfulDiscover() throws Exception {
        DeviceDiscoverTask out = new DeviceDiscoverTask(m_deviceFinder) {
            Document getDiscoveredDevicesXmlDocument() {
                try {
                    return parseDeviceDocument();
                } catch (Exception e) {
                    fail(e.getMessage());
                    return null;
                }
            }
        };
        out.run();
        assertEquals(DeviceFinder.FINISHED, m_deviceFinder.getState());

        List<DiscoveredDevice> discoveredDevices = m_deviceFinder.getDevices();
        assertEquals(3, discoveredDevices.size());

        DiscoveredDevice firstDevice = new DiscoveredDevice();
        firstDevice.setMacAddress("000b820a20e5");
        DiscoveredDevice secondDevice = new DiscoveredDevice();
        secondDevice.setMacAddress("0004f2008742");
        DiscoveredDevice thirdDevice = new DiscoveredDevice();
        thirdDevice.setMacAddress("000413232667");

        assertTrue(discoveredDevices.contains(firstDevice));
        assertTrue(discoveredDevices.contains(secondDevice));
        assertTrue(discoveredDevices.contains(thirdDevice));
    }

    private Document parseDeviceDocument() throws Exception {
        File xmlFile = new File(TestUtil.getTestSourceDirectory(getClass()),"devicesFound.xml");
        String xmlString = FileUtils.readFileToString(xmlFile);
        Document document = DocumentHelper.parseText(xmlString);

        return document;
    }
}
