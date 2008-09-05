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

public class SipxStatusService extends SipxService {
    public static final String BEAN_ID = "sipxStatusService";
    private static final ProcessName PROCESS_NAME = ProcessName.STATUS;
    
    private int m_httpPort;
    private int m_httpsPort;
    private int m_sipPort;
    
    public int getHttpPort() {
        return m_httpPort;
    }

    public void setHttpPort(int httpPort) {
        this.m_httpPort = httpPort;
    }

    public int getHttpsPort() {
        return m_httpsPort;
    }

    public void setHttpsPort(int httpsPort) {
        this.m_httpsPort = httpsPort;
    }

    public int getStatusServerSipPort() {
        return m_sipPort;
    }

    public void setStatusServerSipPort(int sipPort) {
        this.m_sipPort = sipPort;
    }

    @Override
    public ProcessName getProcessName() {
        return PROCESS_NAME;
    }
}
