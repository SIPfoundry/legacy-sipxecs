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

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.DiscoveredDevice;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.phone.polycom.PolycomModel;
import org.springframework.context.ApplicationContext;

public class DiscoveredDeviceManagerTestDb extends TestHelper.TestCaseDb {
    private DiscoveredDeviceManager m_context;
    private ApplicationContext m_appContext;
    private PhoneContext m_phoneContext;
    private GatewayContext m_gatewayContext;

    protected void setUp() throws Exception {
        m_appContext = TestHelper.getApplicationContext();
        m_context = (DiscoveredDeviceManager) m_appContext.getBean(DiscoveredDeviceManager.CONTEXT_BEAN_NAME);
        m_phoneContext = (PhoneContext) m_appContext.getBean(PhoneContext.CONTEXT_BEAN_NAME);
        m_gatewayContext = (GatewayContext) m_appContext.getBean(GatewayContext.CONTEXT_BEAN_NAME);
        m_context.setPhoneContext(m_phoneContext);
        m_context.setGatewayContext(m_gatewayContext);
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testGetDevices() throws Exception {
	TestHelper.insertFlat("phone/discovered_devices.db.xml");
	List<DiscoveredDevice> devices = m_context.getDiscoveredDevices();
	assertEquals(2, devices.size());
    }

    public void testUpdateDevices() throws Exception {
	TestHelper.insertFlat("phone/discovered_devices.db.xml");
	List<DiscoveredDevice> devices = new ArrayList<DiscoveredDevice>();

	//device to be added
	DiscoveredDevice newDevice = new DiscoveredDevice();
	newDevice.setMacAddress("0123456789AB");
	newDevice.setIpAddress("102.102.102.102");
	newDevice.setVendor("testVendor2");
	devices.add(newDevice);
	//device just to be updated
	DiscoveredDevice updatedDevice = new DiscoveredDevice();
	updatedDevice.setMacAddress("001122334455");
	updatedDevice.setIpAddress("201.201.201.201");
	updatedDevice.setVendor("testVendor3");
	devices.add(updatedDevice);

	m_context.updateDiscoveredDevices(devices);

	devices = m_context.getDiscoveredDevices();
	assertEquals(3, devices.size());
	DiscoveredDevice changedDevice = null;
	for(DiscoveredDevice device : devices) {
		if(device.getMacAddress().equalsIgnoreCase("001122334455")) {
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

	    assertEquals("Discovered Device",phone.getDescription());
    }
}
