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
import org.sipfoundry.sipxconfig.phone.PhoneModel;

public class AudioCodesFxsModel extends PhoneModel {
    public static final DeviceVersion[] VERSIONS = {
        AudioCodesModel.REL_6_0,
        AudioCodesModel.REL_5_8,
        AudioCodesModel.REL_5_6,
        AudioCodesModel.REL_5_4,
        AudioCodesModel.REL_5_2,
        AudioCodesModel.REL_5_0
    };

    private String m_configDirectory;

    public AudioCodesFxsModel() {
        setVersions(VERSIONS);
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }

    public String getConfigDirectory() {
        return m_configDirectory;
    }
}
