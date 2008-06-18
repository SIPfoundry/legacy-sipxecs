/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.phone;

import java.util.List;

import org.sipfoundry.sipxconfig.device.DiscoveredDevice;

public interface DiscoveredDeviceManager {
    public List<DiscoveredDevice> getDiscoveredDevices();

    public List<DiscoveredDevice> getUnregisteredDiscoveredDevices();

    public void updateDiscoveredDevices(List<DiscoveredDevice> devices);

    public void saveDiscoveredDevices(List<DiscoveredDevice> devices);
}
