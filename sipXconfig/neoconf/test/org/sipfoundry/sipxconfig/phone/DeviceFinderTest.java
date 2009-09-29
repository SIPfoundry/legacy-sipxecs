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

import junit.framework.TestCase;

import org.apache.commons.io.FileUtils;
import org.dom4j.Document;
import org.dom4j.DocumentHelper;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.device.DiscoveredDevice;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class DeviceFinderTest extends TestCase {

    private static final String MAC_ADDR_3 = "000413232667";
    private static final String MAC_ADDR_2 = "0004f2008742";
    private static final String MAC_ADDR_1 = "000b820a20e5";
    private DeviceFinder m_out;
    private PhoneContext m_phoneContext;
    private GatewayContext m_gatewayContext;

    protected void setUp() throws Exception {
        m_out = new DeviceFinder();

        DiscoverTaskMock discoverTask = new DiscoverTaskMock(m_out);
        m_out.setTask(discoverTask);

        //additional mock configuration and activation (replay) is done in test methods
        m_phoneContext = EasyMock.createNiceMock(PhoneContext.class);
        m_phoneContext.getPhoneIdBySerialNumber(MAC_ADDR_3);
        EasyMock.expectLastCall().andReturn(1).anyTimes();
        m_phoneContext.getPhoneIdBySerialNumber(EasyMock.isA(String.class));
        EasyMock.expectLastCall().andReturn(null).anyTimes();
        m_out.setPhoneContext(m_phoneContext);

        m_gatewayContext = EasyMock.createStrictMock(GatewayContext.class);
        m_gatewayContext.getGatewayIdBySerialNumber(EasyMock.isA(String.class));
        EasyMock.expectLastCall().andReturn(null).anyTimes();
        m_out.setGatewayContext(m_gatewayContext);
    }

    public void testGetDevices() {
        EasyMock.replay(m_phoneContext);
        EasyMock.replay(m_gatewayContext);

        assertEquals(m_out.getState(), DeviceFinder.NOT_STARTED);
        m_out.start();

        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        assertEquals(DeviceFinder.FINISHED, m_out.getState());
        List<DiscoveredDevice> devices = m_out.getDevices();
        assertEquals(3, devices.size());

        DiscoveredDevice device1 = devices.get(0);
        assertEquals("192.168.1.253", device1.getIpAddress());
        assertEquals(MAC_ADDR_1, device1.getMacAddress());
        assertEquals("Grandstream", device1.getVendor());

        DiscoveredDevice device2 = devices.get(1);
        assertEquals("192.168.1.252", device2.getIpAddress());
        assertEquals(MAC_ADDR_2, device2.getMacAddress());
        assertEquals("Polycom", device2.getVendor());

        DiscoveredDevice device3 = devices.get(2);
        assertEquals("192.168.1.254", device3.getIpAddress());
        assertEquals(MAC_ADDR_3, device3.getMacAddress());
        assertEquals("SNOM", device3.getVendor());

        assertEquals(DeviceFinder.FINISHED, m_out.getState());
    }

    public void testGetDiscoveredDevices() {
        EasyMock.replay(m_phoneContext);
        EasyMock.replay(m_gatewayContext);

        assertEquals(m_out.getState(), DeviceFinder.NOT_STARTED);
        m_out.start();

        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        assertEquals(DeviceFinder.FINISHED, m_out.getState());
        List<DiscoveredDevice> devices = m_out.getDiscoveredDevices();
        assertEquals(2, devices.size());

        DiscoveredDevice device1 = devices.get(0);
        assertEquals("192.168.1.253", device1.getIpAddress());
        assertEquals(MAC_ADDR_1, device1.getMacAddress());
        assertEquals("Grandstream", device1.getVendor());

        DiscoveredDevice device2 = devices.get(1);
        assertEquals("192.168.1.252", device2.getIpAddress());
        assertEquals(MAC_ADDR_2, device2.getMacAddress());
        assertEquals("Polycom", device2.getVendor());

        assertEquals(DeviceFinder.FINISHED, m_out.getState());
    }

    public void testSaveDiscoveredDevices() {
        m_phoneContext.storePhone(EasyMock.isA(Phone.class));
        EasyMock.expectLastCall().anyTimes();
        EasyMock.replay(m_phoneContext);

        EasyMock.replay(m_gatewayContext);

        m_out.start();
        EasyMock.verify(m_phoneContext);
        EasyMock.verify(m_gatewayContext);
    }

    class DiscoverTaskMock extends DeviceDiscoverTask {
        public DiscoverTaskMock(DeviceFinder deviceFinder) {
            super(deviceFinder);
        }
        Document getDiscoveredDevicesXmlDocument() {
            Document document = null;
            try {
                File xmlFile = new File(TestUtil.getTestSourceDirectory(getClass()),
                    "devicesFound.xml");
                String xmlString = FileUtils.readFileToString(xmlFile);
                document = DocumentHelper.parseText(xmlString);
            } catch (Exception e) {
                e.printStackTrace();
            }
            return document;
        }
    }
}
