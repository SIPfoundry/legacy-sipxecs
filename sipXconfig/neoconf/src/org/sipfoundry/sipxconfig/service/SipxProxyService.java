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

public class SipxProxyService extends SipxService {
    public static final String BEAN_ID = "sipxProxyService";

    private String m_secureSipPort;
    private String m_callResolverCallStateDb;

    public String getSecureSipPort() {
        return m_secureSipPort;
    }

    public void setSecureSipPort(String secureSipPort) {
        this.m_secureSipPort = secureSipPort;
    }

    public String getCallResolverCallStateDb() {
        return m_callResolverCallStateDb;
    }

    public void setCallResolverCallStateDb(String callResolverCallStateDb) {
        this.m_callResolverCallStateDb = callResolverCallStateDb;
    }

    @Override
    public String getBeanId() {
        return BEAN_ID;
    }
}
