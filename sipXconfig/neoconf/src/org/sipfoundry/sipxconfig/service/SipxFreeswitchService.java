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

import org.springframework.beans.factory.annotation.Required;

public class SipxFreeswitchService extends SipxService implements LoggingEntity {
    public static final String BEAN_ID = "sipxFreeswitchService";

    public static final String LOG_SETTING = "freeswitch-config/FREESWITCH_SIP_DEBUG";

    private static final String FALSE = "0";
    private static final String TRUE = "1";
    private static final String DEBUG = "DEBUG";
    private static final String INFO = "INFO";
    private static final String NON_DEBUG = "NON-DEBUG";

    private String m_docDir;

    @Required
    public void setDocDir(String docDir) {
        m_docDir = docDir;
    }

    public String getDocDir() {
        return m_docDir;
    }

    @Override
    public String getLogSetting() {
        return LOG_SETTING;
    }

    @Override
    public void setLogLevel(String logLevel) {
        if (logLevel != null && (logLevel.equals(DEBUG) || logLevel.equals(INFO))) {
            super.setLogLevel(TRUE);
        } else {
            super.setLogLevel(FALSE);
        }
    }

    @Override
    public String getLogLevel() {
        String logLevel = super.getLogLevel();
        if (logLevel != null && logLevel.equals(TRUE)) {
            return DEBUG;
        } else {
            return NON_DEBUG;
        }
    }

    @Override
    public String getLabelKey() {
        return super.getLabelKey();
    }
}
