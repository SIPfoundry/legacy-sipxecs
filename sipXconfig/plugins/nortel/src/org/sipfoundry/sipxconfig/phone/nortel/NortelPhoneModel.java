/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.phone.nortel;

import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.phone.PhoneModel;

public class NortelPhoneModel extends PhoneModel {
    public static final String VENDOR = "nortelPhone";
    public static final DeviceVersion FIRM_2_2 = new DeviceVersion(VENDOR, "2.2");
    public static final DeviceVersion FIRM_3_2 = new DeviceVersion(VENDOR, "3.2");

    public NortelPhoneModel() {
        super();
        setVersions(getDeviceVersions());
    }

    public static DeviceVersion[] getDeviceVersions() {
        return new DeviceVersion[] {
            FIRM_2_2, FIRM_3_2
        };
    }
}
