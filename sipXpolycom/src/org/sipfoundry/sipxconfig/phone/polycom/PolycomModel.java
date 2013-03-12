/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.phone.PhoneModel;

/**
 * Static differences in polycom phone models
 */
public final class PolycomModel extends PhoneModel {

    /** Firmware 2.0 or beyond */
    public static final DeviceVersion VER_2_0 = new DeviceVersion(PolycomPhone.BEAN_ID, "2.0");
    public static final DeviceVersion VER_3_1_X = new DeviceVersion(PolycomPhone.BEAN_ID, "3.1.X");
    public static final DeviceVersion VER_3_2_X = new DeviceVersion(PolycomPhone.BEAN_ID, "3.2.X");
    public static final DeviceVersion VER_4_0_X = new DeviceVersion(PolycomPhone.BEAN_ID, "4.0.X");
    public static final DeviceVersion VER_4_1_X = new DeviceVersion(PolycomPhone.BEAN_ID, "4.1.X");
    public static final DeviceVersion[] SUPPORTED_VERSIONS = new DeviceVersion[] {
        VER_3_1_X, VER_3_2_X, VER_4_0_X, VER_4_1_X
    };

    public PolycomModel() {
        super(PolycomPhone.BEAN_ID);
        setEmergencyConfigurable(true);
    }

    public static DeviceVersion getPhoneDeviceVersion(String version) {
        for (DeviceVersion deviceVersion : SUPPORTED_VERSIONS) {
            if (deviceVersion.getName().contains(version)) {
                return deviceVersion;
            }
        }
        return VER_2_0;
    }

}
