/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxStatusService extends SipxService {
    public static final String BEAN_ID = "sipxStatusService";

    // TODO: remove once it's removed from config.defs and voicemail.xml
    private int m_httpsPort;

    public void setHttpsPort(int httpsPort) {
        m_httpsPort = httpsPort;
    }

    public int getHttpsPort() {
        return m_httpsPort;
    }

    @Override
    public String getSipPort() {
        Setting statusSettings = getSettings().getSetting("status-config");
        return statusSettings.getSetting("SIP_STATUS_SIP_PORT").getValue();
    }
}
