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

import org.springframework.dao.support.DataAccessUtils;

public class SipxRlsService extends SipxService implements LoggingEntity {
    public static final String BEAN_ID = "sipxRlsService";

    public static final String LOG_SETTING = "rls-config/SIP_RLS_LOG_LEVEL";

    private String m_rlsPort;

    public String getRlsPort() {
        return m_rlsPort;
    }

    public void setRlsPort(String rlsPort) {
        m_rlsPort = rlsPort;
    }

    public String getAddress() {
        String address = (String) DataAccessUtils.singleResult(getAddresses());
        return null == address ? "" : address;
    }

    @Override
    public String getLogSetting() {
        return LOG_SETTING;
    }

    @Override
    public void setLogLevel(String logLevel) {
        super.setLogLevel(logLevel);
    }

    @Override
    public String getLogLevel() {
        return super.getLogLevel();
    }

    @Override
    public String getLabelKey() {
        return super.getLabelKey();
    }
}
