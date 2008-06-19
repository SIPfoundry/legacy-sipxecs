/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.phone;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.device.DiscoveredDevice;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.gateway.GatewayModel;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class DiscoveredDeviceManagerImpl extends SipxHibernateDaoSupport<DiscoveredDevice> implements
        DiscoveredDeviceManager {

    private static final String DISCOVERED_DEVICE = "Discovered Device";

    private static final String SEPARATOR = " - ";

    private PhoneContext m_phoneContext;

    private GatewayContext m_gatewayContext;

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public void setGatewayContext(GatewayContext gatewayContext) {
        m_gatewayContext = gatewayContext;
    }

    public List<DiscoveredDevice> getDiscoveredDevices() {
        List<DiscoveredDevice> devices = getHibernateTemplate().loadAll(DiscoveredDevice.class);
        return devices;
    }

    public List<DiscoveredDevice> getUnsavedDiscoveredDevices() {
        List<DiscoveredDevice> unregisteredDevices = new ArrayList<DiscoveredDevice>();
        List<DiscoveredDevice> discoveredDevices = getDiscoveredDevices();
        for (DiscoveredDevice device : discoveredDevices) {
            String macAddress = device.getMacAddress();
            if (m_phoneContext.getPhoneIdBySerialNumber(macAddress) == null
                    && m_gatewayContext.getGatewayIdBySerialNumber(macAddress) == null) {
                unregisteredDevices.add(device);
            }
        }
        return unregisteredDevices;
    }

    public void updateDiscoveredDevices(List<DiscoveredDevice> devices) {
        HibernateTemplate template = getHibernateTemplate();
        for (DiscoveredDevice device : devices) {
            Object dbObject = template.get(DiscoveredDevice.class, device.getMacAddress());
            if (dbObject == null) {
                template.save(device);
            } else {
                DiscoveredDevice dbDevice = (DiscoveredDevice) dbObject;
                dbDevice.setIpAddress(device.getIpAddress());
                dbDevice.setVendor(device.getVendor());
                template.update(dbDevice);
            }
        }
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
                gateway.setName(device.getMacAddress() + SEPARATOR + DISCOVERED_DEVICE);
                m_gatewayContext.storeGateway(gateway);
            }
        }
    }
}
