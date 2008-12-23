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

public class SipxPageService extends SipxService {

    public static final String BEAN_ID = "sipxPageService";
    
    private static final String SIP_PORT = "page-config/PAGE_SERVER_SIP_PORT";

    private String m_audioDirectory;

    public String getAudioDir() {
        return m_audioDirectory;
    }

    public void setAudioDir(String audioDirectory) {
        m_audioDirectory = audioDirectory;
    }
    public String getSipPort() {
        return getSettingValue(SIP_PORT);
    }
}
