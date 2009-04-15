/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

public class SipxRelayService extends SipxService implements LoggingEntity {
    public static final String BEAN_ID = "sipxRelayService";

    public static final String LOG_SETTING = "relay-config/SIP_RELAY_LOG_LEVEL";

    public String getXmlRpcPort() {
        return getSettingValue("relay-config/xml-rpc-port");
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
