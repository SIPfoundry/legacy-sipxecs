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

    private String m_audioDirectory;
    
    public String getAudioDir() {
        return m_audioDirectory;
    }

    public void setAudioDir(String audioDirectory) {
        m_audioDirectory = audioDirectory;
    }

    @Override
    public String getBeanId() {
        return BEAN_ID;
    }
}
