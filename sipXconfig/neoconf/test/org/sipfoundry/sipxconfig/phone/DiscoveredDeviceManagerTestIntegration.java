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

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.device.DiscoveredDevice;
import org.sipfoundry.sipxconfig.phone.polycom.PolycomModel;

public class DiscoveredDeviceManagerTestIntegration extends IntegrationTestCase {
    private DiscoveredDeviceManager m_context;
    private PhoneContext m_phoneContext;

    public void setDiscoveredDeviceManager(DiscoveredDeviceManager context) {
        m_context = context;
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public void testGetDevices() throws Exception {
        loadDataSet("phone/discovered_devices.db.xml");
        List<DiscoveredDevice> devices = m_context.getDiscoveredDevices();
        assertEquals(2, devices.size());
    }

    public void testGetUnsavedDevices() throws Exception {
        loadDataSet("phone/discovered_devices.db.xml");
        List<DiscoveredDevice> devices = m_context.getUnsavedDiscoveredDevices();
        assertEquals(1, devices.size());
        DiscoveredDevice device1 = devices.get(0);
        assertEquals("000011112222", device1.getMacAddress());
    }

    public void testUpdateDevices() throws Exception {
        loadDataSet("phone/discovered_devices.db.xml");
        List<DiscoveredDevice> devices = new ArrayList<DiscoveredDevice>();

        // device to be added
        DiscoveredDevice newDevice = new DiscoveredDevice();
        newDevice.setMacAddress("0123456789AB");
        newDevice.setIpAddress("102.102.102.102");
        newDevice.setVendor("testVendor2");
        devices.add(newDevice);
        // device just to be updated
        DiscoveredDevice updatedDevice = new DiscoveredDevice();
        updatedDevice.setMacAddress("001122334455");
        updatedDevice.setIpAddress("201.201.201.201");
        updatedDevice.setVendor("testVendor3");
        devices.add(updatedDevice);

        m_context.updateDiscoveredDevices(devices);

        devices = m_context.getDiscoveredDevices();
        assertEquals(3, devices.size());
        DiscoveredDevice changedDevice = null;
        for (DiscoveredDevice device : devices) {
            if (device.getMacAddress().equalsIgnoreCase("001122334455")) {
                changedDevice = device;
            }
        }
        assertEquals("201.201.201.201", changedDevice.getIpAddress());
        assertEquals("testVendor3", changedDevice.getVendor());
    }

    public void testSaveDevices() throws Exception {
        List<DiscoveredDevice> devices = new ArrayList<DiscoveredDevice>();
        DiscoveredDevice device = new DiscoveredDevice();
        device.setMacAddress("000011112222");
        device.setIpAddress("100.100.100.100");
        device.setVendor("Polycom");
        PhoneModel model = new PolycomModel();
        model.setModelId("polycom300");
        device.setModel(model);
        devices.add(device);
        m_context.saveDiscoveredDevices(devices);

        Integer phoneId = m_phoneContext.getPhoneIdBySerialNumber("000011112222");
        Phone phone = m_phoneContext.loadPhone(phoneId);

        assertEquals("Discovered Device", phone.getDescription());
    }
}
