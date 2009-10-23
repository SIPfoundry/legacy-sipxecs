/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.service;

public class SipxProvisionService extends SipxService implements LoggingEntity {

    public static final String BEAN_ID = "sipxProvisionService";
    public static final String LOG_SETTING = "provision-config/SIPX_PROV_LOG_LEVEL";
    private String m_tftproot;


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

    public String getTftproot() {
        return m_tftproot;
    }

    public void setTftproot(String tftproot) {
        m_tftproot = tftproot;
    }
}
