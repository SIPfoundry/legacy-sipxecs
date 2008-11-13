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

import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel.ProcessName;

public class SipxPageService extends SipxService {

    public static final String BEAN_ID = "sipxPageService";
    private static final ProcessName PROCESS_NAME = ProcessName.PAGE_SERVER;
    private String m_audioDirectory;
    
    @Override
    public ProcessName getProcessName() {
        return PROCESS_NAME;
    }

    public String getAudioDir() {
        return m_audioDirectory;
    }

    public void setAudioDir(String audioDirectory) {
        m_audioDirectory = audioDirectory;
    }

}
