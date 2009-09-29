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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.device.DiscoveredDevice;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.gateway.GatewayModel;
import org.springframework.core.task.SimpleAsyncTaskExecutor;

public class DeviceFinder {
    public static final String RUNNING = "Running";

    public static final String FINISHED = "Finished";

    public static final String FAILED = "Failed";

    public static final String NOT_STARTED = "Not Started";

    static final Log LOG = LogFactory.getLog(DeviceFinder.class);

    private static final String DISCOVERED_DEVICE = "Discovered Device";

    private static final String SPACE_MINUS = " - ";

    private PhoneContext m_phoneContext;

    private GatewayContext m_gatewayContext;

    private String m_binDirectory;

    private SimpleAsyncTaskExecutor m_taskExecutor = new SimpleAsyncTaskExecutor();

    private String m_state = NOT_STARTED;

    private List<DiscoveredDevice> m_discoveredDevices;

    private Runnable m_task = new DeviceDiscoverTask(this);

    public void setBinDirectory(String binDirectory) {
        m_binDirectory = binDirectory;
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public void setGatewayContext(GatewayContext gatewayContext) {
        m_gatewayContext = gatewayContext;
    }

    public Runnable getTask() {
        return m_task;
    }

    public void setTask(Runnable task) {
        m_task = task;
    }

    /**
     * Gets the list of all discovered devices
     * @return
     */
    public List<DiscoveredDevice> getDevices() {
        return m_discoveredDevices;
    }

    public void setDevices(List<DiscoveredDevice> discoveredDevices) {
        m_discoveredDevices = discoveredDevices;
    }

    /**
     * Gets the list of discovered devices that are not already registered as phones
     * or gateways with sipXconfig
     * @return
     */
    public List<DiscoveredDevice> getDiscoveredDevices() {
        List<DiscoveredDevice> unregisteredDevices = new ArrayList<DiscoveredDevice>();
        for (DiscoveredDevice device : m_discoveredDevices) {
            String macAddress = device.getMacAddress();
            if (m_phoneContext.getPhoneIdBySerialNumber(macAddress) == null
                    && m_gatewayContext.getGatewayIdBySerialNumber(macAddress) == null) {
                unregisteredDevices.add(device);
            }
        }
        return unregisteredDevices;
    }

    public String getState() {
        return m_state;
    }

    public void setState(String state) {
        m_state = state;
    }

    public void saveDiscoveredDevices(List<DiscoveredDevice> devices) {
        for (DiscoveredDevice device : devices) {
            if (PhoneModel.class.isAssignableFrom(device.getModel().getClass())) {
                Phone phone = m_phoneContext.newPhone((PhoneModel) device.getModel());
                phone.setSerialNumber(device.getMacAddress());
                phone.setDescription(DISCOVERED_DEVICE);
                m_phoneContext.storePhone(phone);
            } else if (GatewayModel.class.isAssignableFrom(device.getModel().getClass())) {
                Gateway gateway = m_gatewayContext.newGateway((GatewayModel) device.getModel());
                gateway.setSerialNumber(device.getMacAddress());
                gateway.setDescription(DISCOVERED_DEVICE);
                gateway.setAddress(device.getIpAddress());
                gateway.setName(device.getMacAddress() + SPACE_MINUS + DISCOVERED_DEVICE);
                m_gatewayContext.storeGateway(gateway);
            }
        }
    }

    public void start() {
        if (!getState().equals(RUNNING)) {
            m_state = RUNNING;
            m_taskExecutor.execute(m_task);
        }
    }

    String getBinDirectory() {
        return m_binDirectory;
    }
}
