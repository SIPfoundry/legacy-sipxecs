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

public class SipxRestService extends SipxService implements LoggingEntity {
    public static final String BEAN_ID = "sipxRestService";

    public static final String LOG_SETTING = "rest-config/SIPX_REST_LOG_LEVEL";

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

    public String getExtHttpPort() {
        return getSettingValue("rest-config/extHttpPort");
    }

    public String getHttpsPort() {
        return getSettingValue("rest-config/httpsPort");
    }
}
