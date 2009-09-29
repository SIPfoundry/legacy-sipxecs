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

import org.sipfoundry.sipxconfig.common.PrimaryKeySource;

public class DiscoveredDevice implements PrimaryKeySource {

    private String m_macAddress;

    private String m_ipAddress;

    private String m_vendor;

    private DeviceDescriptor m_model;

    public String getMacAddress() {
        return m_macAddress;
    }

    public String getIpAddress() {
        return m_ipAddress;
    }

    public String getVendor() {
        return m_vendor;
    }

    public DeviceDescriptor getModel() {
        return m_model;
    }

    public void setMacAddress(String macAddress) {
        m_macAddress = macAddress;
    }

    public void setIpAddress(String ipAddress) {
        m_ipAddress = ipAddress;
    }

    public void setVendor(String vendor) {
        m_vendor = vendor;
    }

    public void setModel(DeviceDescriptor model) {
        m_model = model;
    }

    public Object getPrimaryKey() {
        return getMacAddress();
    }

    /**
     * Returns true if the specified object is an instance of DiscoveredDevice
     * and has a MAC address that matches the MAC address of this instance
     */
    public boolean equals(Object o) {
        if (!(o instanceof DiscoveredDevice)) {
            return false;
        }
        DiscoveredDevice device = (DiscoveredDevice) o;
        if (device.getMacAddress().equals(getMacAddress())) {
            return true;
        }
        return false;
    }

    public int hashCode() {
        return m_macAddress.hashCode();
    }
}
