/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.api;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.device.DiscoveredDevice;
import org.sipfoundry.sipxconfig.phone.DiscoveredDeviceManager;

public class DeviceDiscoveryServiceImpl implements DeviceDiscoveryService {

    private DiscoveredDeviceBuilder m_builder;

    private DiscoveredDeviceManager m_discoveredDeviceManager;

    public void setDiscoveredDeviceBuilder(DiscoveredDeviceBuilder builder) {
        m_builder = builder;
    }

    public void setDiscoveredDeviceManager(DiscoveredDeviceManager discoveredDeviceManager) {
        m_discoveredDeviceManager = discoveredDeviceManager;
    }

    public void updateDiscoveredDevicesList(UpdateDiscoveredDevicesList request) {
        org.sipfoundry.sipxconfig.api.DiscoveredDevice[] apiDevices = request.getUpdatedDevices();
        DiscoveredDevice[] myDevices = (DiscoveredDevice[]) ApiBeanUtil.toMyArray(m_builder,
                apiDevices, DiscoveredDevice.class);
        List<DiscoveredDevice> myDevicesList = new ArrayList<DiscoveredDevice>();
        for (DiscoveredDevice myDevice : myDevices) {
            myDevicesList.add(myDevice);
        }
        m_discoveredDeviceManager.updateDiscoveredDevices(myDevicesList);
    }

    public RetrieveDiscoveredDevicesList retrieveDiscoveredDevicesList() {
        List<DiscoveredDevice> myDevicesList = m_discoveredDeviceManager.getDiscoveredDevices();
        DiscoveredDevice[] myDevices = new DiscoveredDevice[0];
        myDevices = myDevicesList.toArray(myDevices);
        org.sipfoundry.sipxconfig.api.DiscoveredDevice[] apiDevices =
            (org.sipfoundry.sipxconfig.api.DiscoveredDevice[]) ApiBeanUtil.toApiArray(m_builder, myDevices,
                    org.sipfoundry.sipxconfig.api.DiscoveredDevice.class);
        RetrieveDiscoveredDevicesList retrievedDevices = new RetrieveDiscoveredDevicesList();
        retrievedDevices.setRetrievedDevices(apiDevices);
        return retrievedDevices;
    }
}
