/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mwi;

import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class MwiSettings extends BeanWithSettings {

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxstatus/sipxstatus.xml");
    }

    public int getTcp() {
        return ((Integer) getSettingTypedValue("status-config/SIP_STATUS_TCP_PORT")).intValue();
    }

    public int getUdpPort() {
        return ((Integer) getSettingTypedValue("status-config/SIP_STATUS_UDP_PORT")).intValue();
    }

    public int getHttpApiPort() {
        return ((Integer) getSettingTypedValue("status-config/SIP_STATUS_HTTP_PORT")).intValue();
    }

    public int getHttpsApiPort() {
        return ((Integer) getSettingTypedValue("status-config/SIP_STATUS_HTTPS_PORT")).intValue();
    }
}
