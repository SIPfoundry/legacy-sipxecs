/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.api;

import java.util.Arrays;
import java.util.Set;

public class DiscoveredDeviceBuilder extends SimpleBeanBuilder {
    private static final String MAC_ADDRESS_PROP = "macAddress";
    private static final String IP_ADDRESS_PROP = "ipAddress";
    private static final String VENDOR_PROP = "vendor";
    private static final String LAST_SEEN_PROP = "lastSeen";

    private static final String[] CUSTOM_FIELDS = {
        MAC_ADDRESS_PROP, IP_ADDRESS_PROP, VENDOR_PROP, LAST_SEEN_PROP
    };

    public DiscoveredDeviceBuilder() {
        getCustomFields().addAll(Arrays.asList(CUSTOM_FIELDS));
    }

    public void toApiObject(Object apiObject, Object myObject, Set properties) {
        super.toApiObject(apiObject, myObject, properties);
        DiscoveredDevice device = (DiscoveredDevice) apiObject;
        org.sipfoundry.sipxconfig.device.DiscoveredDevice otherDevice =
            (org.sipfoundry.sipxconfig.device.DiscoveredDevice) myObject;
        device.setMacAddress(otherDevice.getMacAddress());
        device.setIpAddress(otherDevice.getIpAddress());
        device.setVendor(otherDevice.getVendor());
        device.setLastSeen(otherDevice.getLastSeen());
    }

    public void toMyObject(Object myObject, Object apiObject, Set properties) {
        super.toMyObject(myObject, apiObject, properties);
        DiscoveredDevice device = (DiscoveredDevice) apiObject;
        org.sipfoundry.sipxconfig.device.DiscoveredDevice otherDevice =
            (org.sipfoundry.sipxconfig.device.DiscoveredDevice) myObject;
        otherDevice.setMacAddress(device.getMacAddress());
        otherDevice.setIpAddress(device.getIpAddress());
        otherDevice.setVendor(device.getVendor());
        otherDevice.setLastSeen(device.getLastSeen());
    }
}
