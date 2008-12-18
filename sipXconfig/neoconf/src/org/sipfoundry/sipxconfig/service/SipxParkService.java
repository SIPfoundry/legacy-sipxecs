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

public class SipxParkService extends SipxService {
    public static final String BEAN_ID = "sipxParkService";
    private static final ProcessName PROCESS_NAME = ProcessName.PARK_SERVER;
    
    private String m_parkServerSipPort;
    
    public String getParkServerSipPort() {
        return m_parkServerSipPort;
    }
    
    public void setParkServerSipPort(String parkServerSipPort) {
        m_parkServerSipPort = parkServerSipPort;
    }

    @Override
    public ProcessName getProcessName() {
        return PROCESS_NAME;
    }

    @Override
    public String getBeanId() {
        return BEAN_ID;
    }
}
