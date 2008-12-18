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

public class SipxRlsService extends SipxService {
    public static final String BEAN_ID = "sipxRlsService";
    private String m_rlsPort;

    public String getRlsPort() {
        return m_rlsPort;
    }

    public void setRlsPort(String rlsPort) {
        m_rlsPort = rlsPort;
    }

    @Override
    public String getBeanId() {
        return BEAN_ID;
    }
}
