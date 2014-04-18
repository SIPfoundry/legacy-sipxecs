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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
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
    public static final DeviceVersion VER_3_3_X = new DeviceVersion(PolycomPhone.BEAN_ID, "3.3.X");
    public static final DeviceVersion VER_4_0_X = new DeviceVersion(PolycomPhone.BEAN_ID, "4.0.X");
    public static final DeviceVersion VER_4_1_X = new DeviceVersion(PolycomPhone.BEAN_ID, "4.1.X");
    public static final DeviceVersion VER_4_1_0 = new DeviceVersion(PolycomPhone.BEAN_ID, "4.1.0");
    public static final DeviceVersion VER_4_1_2 = new DeviceVersion(PolycomPhone.BEAN_ID, "4.1.2");
    public static final DeviceVersion VER_4_1_3 = new DeviceVersion(PolycomPhone.BEAN_ID, "4.1.3");
    public static final DeviceVersion VER_4_1_4 = new DeviceVersion(PolycomPhone.BEAN_ID, "4.1.4");
    public static final DeviceVersion VER_4_1_5 = new DeviceVersion(PolycomPhone.BEAN_ID, "4.1.5");
    public static final DeviceVersion VER_5_0_0 = new DeviceVersion(PolycomPhone.BEAN_ID, "5.0.0");
    public static final DeviceVersion VER_5_0_1 = new DeviceVersion(PolycomPhone.BEAN_ID, "5.0.1");
    public static final DeviceVersion VER_5_0_2 = new DeviceVersion(PolycomPhone.BEAN_ID, "5.0.2");
    public static final DeviceVersion[] SUPPORTED_VERSIONS = new DeviceVersion[] {
        VER_3_1_X, VER_3_2_X, VER_4_0_X, VER_4_1_X, VER_4_1_0, VER_4_1_2, VER_4_1_3, VER_4_1_4, VER_4_1_5,
        VER_5_0_0, VER_5_0_1, VER_5_0_2
    };
    private static final Log LOG = LogFactory.getLog(PolycomModel.class);
    private DeviceVersion m_deviceVersion;

    public PolycomModel() {
        super(PolycomPhone.BEAN_ID);
        setEmergencyConfigurable(true);
    }

    /**
     * checks if this version is greater than 4.0
     *
     * @return
     */
    protected static boolean is40orLater(DeviceVersion v) {
        return PolycomModel.compareVersions(v, new Integer[] {
            4, 0
        }) >= 0;
    }

    /**
     * generic method to compare versions
     *
     * @param deviceVersion
     * @param testVersion
     * @return negative if test version is greater than device version; positive if it is smaller
     *         and 0 if it they are equal
     */
    protected static int compareVersions(DeviceVersion deviceVersion, Integer[] testVersion) {
        DeviceVersion deviceVersionCopy = deviceVersion;
        if (deviceVersionCopy == null) {
            // This is wrong!! LOG AN ERROR, but assume it's about 3.3
            deviceVersionCopy = PolycomModel.VER_3_3_X;
            LOG.error("Phone has NULL version id. It might be that it is has 3.3 firmware. Please correct the db!");
        }
        String versionId = deviceVersionCopy.getVersionId();
        String[] tokens = versionId.split("\\.");
        for (int i = 0; i < testVersion.length; i++) {
            Integer ver;
            Integer test = testVersion[i];
            if (tokens[i].equals("X")) {
                ver = 0;
            } else {
                ver = Integer.parseInt(tokens[i]);
            }
            if (ver != test) {
                return ver - test;
            }
        }
        return 0;
    }

    public static DeviceVersion getPhoneDeviceVersion(String version) {
        for (DeviceVersion deviceVersion : SUPPORTED_VERSIONS) {
            if (deviceVersion.getName().contains(version)) {
                return deviceVersion;
            }
        }
        return VER_2_0;
    }

    public void setDefaultVersion(DeviceVersion ver) {
        m_deviceVersion = ver;
    }

    public DeviceVersion getDefaultVersion() {
        return m_deviceVersion;
    }
}
