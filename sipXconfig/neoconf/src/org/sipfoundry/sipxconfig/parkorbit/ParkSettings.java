/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.parkorbit;

import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class ParkSettings extends PersistableSettings {

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxpark/sipxpark.xml");
    }

    public int getSipTcpPort() {
        return (Integer) getSettingTypedValue("park-config/SIP_PARK_TCP_PORT");
    }

    public int getSipUdpPort() {
        return (Integer) getSettingTypedValue("park-config/SIP_PARK_UDP_PORT");
    }

    public int getRtpPort() {
        return (Integer) getSettingTypedValue("park-config/SIP_PARK_RTP_PORT");
    }

    @Override
    public String getBeanId() {
        return "parkSettings";
    }
}
