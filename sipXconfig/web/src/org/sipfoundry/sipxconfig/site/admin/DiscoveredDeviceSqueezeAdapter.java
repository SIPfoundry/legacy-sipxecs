/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.sipfoundry.sipxconfig.device.DiscoveredDevice;

public class DiscoveredDeviceSqueezeAdapter implements IPrimaryKeyConverter {

    public Object getPrimaryKey(Object value) {
        DiscoveredDevice device = (DiscoveredDevice) value;
        return device.getMacAddress();
    }

    public Object getValue(Object primaryKey) {
        DiscoveredDevice device = new DiscoveredDevice();
        device.setMacAddress(primaryKey.toString());
        return device;
    }
}
