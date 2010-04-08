/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.gateway.GatewayModel;

public class AudioCodesModel extends GatewayModel {
    public static final DeviceVersion REL_5_0 = new DeviceVersion(AudioCodesGateway.BEAN_ID, "5.0");
    public static final DeviceVersion REL_5_2 = new DeviceVersion(AudioCodesGateway.BEAN_ID, "5.2");
    public static final DeviceVersion REL_5_4 = new DeviceVersion(AudioCodesGateway.BEAN_ID, "5.4");
    public static final DeviceVersion REL_5_6 = new DeviceVersion(AudioCodesGateway.BEAN_ID, "5.6");
    public static final DeviceVersion REL_6_0 = new DeviceVersion(AudioCodesGateway.BEAN_ID, "6.0");

    private String m_configDirectory;

    public AudioCodesModel() {
        super(AudioCodesGateway.BEAN_ID);
        setVersions(getDeviceVersions());
    }

    public static DeviceVersion[] getDeviceVersions() {
        return new DeviceVersion[] {
            REL_6_0, REL_5_6, REL_5_4, REL_5_2, REL_5_0
        };
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }

    public String getConfigDirectory() {
        return m_configDirectory;
    }
}
