/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.service;

public class SipxProxyService extends SipxService {
    public static final String BEAN_ID = "sipxProxyService";
    
    private String m_ipAddress;
    private String m_sipPort;
    private String m_secureSipPort;
    private String m_hostname;
    private String m_fullHostname;
    private String m_domainName;
    private String m_realm;
    private String m_callResolverCallStateDb;
    private String m_sipSrvOrHostport;
    
    public String getIpAddress() {
        return m_ipAddress;
    }
    public void setIpAddress(String ipAddress) {
        this.m_ipAddress = ipAddress;
    }
    public String getSipPort() {
        return m_sipPort;
    }
    public void setSipPort(String sipPort) {
        this.m_sipPort = sipPort;
    }
    public String getSecureSipPort() {
        return m_secureSipPort;
    }
    public void setSecureSipPort(String secureSipPort) {
        this.m_secureSipPort = secureSipPort;
    }
    public String getHostname() {
        return m_hostname;
    }
    public void setHostname(String hostname) {
        this.m_hostname = hostname;
    }
    public String getFullHostname() {
        return m_fullHostname;
    }
    public void setFullHostname(String fullHostname) {
        this.m_fullHostname = fullHostname;
    }
    public String getDomainName() {
        return m_domainName;
    }
    public void setDomainName(String domainName) {
        this.m_domainName = domainName;
    }
    public String getRealm() {
        return m_realm;
    }
    public void setRealm(String realm) {
        this.m_realm = realm;
    }
    public String getCallResolverCallStateDb() {
        return m_callResolverCallStateDb;
    }
    public void setCallResolverCallStateDb(String callResolverCallStateDb) {
        this.m_callResolverCallStateDb = callResolverCallStateDb;
    }
    public String getSipSrvOrHostport() {
        return m_sipSrvOrHostport;
    }
    public void setSipSrvOrHostport(String sipSrvOrHostport) {
        this.m_sipSrvOrHostport = sipSrvOrHostport;
    }
    
}
