/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.util.Collection;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.commserver.SipxServer;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.InternalRule;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.ConfiguredService;
import org.sipfoundry.sipxconfig.service.ServiceDescriptor;
import org.sipfoundry.sipxconfig.service.ServiceManager;
import org.sipfoundry.sipxconfig.service.UnmanagedService;

/**
 * Sets up phone and line objects with system defaults.
 */
public class DeviceDefaults {

    private static final String DEFAULT_SIP_PORT = "5060";

    private String m_tftpServer;

    private String m_profileRootUrl;

    private String m_fullyQualifiedDomainName;

    private String m_authorizationRealm;

    private DialPlanContext m_dialPlanContext;

    private DomainManager m_domainManager;

    private SipxServer m_sipxServer;

    /** see config.defs PROXY_SERVER_ADDR */
    private String m_proxyServerAddr;

    /** see config.defs PROXY_SERVER_SIP_PORT */
    private String m_proxyServerSipPort;

    private DeviceTimeZone m_timeZone = new DeviceTimeZone();
    
    private ServiceManager m_serviceManager;
    
    private String m_defaultNtpService = "pool.ntp.org";

    public void setDefaultNtpService(String defaultNtpService) {
        m_defaultNtpService = defaultNtpService;
    }

    public void setServiceManager(ServiceManager serviceManager) {
        m_serviceManager = serviceManager;
    }

    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    public String getDomainName() {
        return m_domainManager.getDomain().getName();
    }
    
    public String getNtpServer() {
        String server = getServer(0, UnmanagedService.NTP);
        return server == null ? m_defaultNtpService : server; 
    }
    
    /**
     * @return null if not set
     */
    public String getAlternateNtpServer() {
        String server = getServer(1, UnmanagedService.NTP);
        return server;
    }
    
    private String getServer(int index, ServiceDescriptor s) {
        String service = null;
        Collection<ConfiguredService> servers = m_serviceManager.getEnabledServicesByType(s);
        if (servers != null && servers.size() > index) {
            service = servers.iterator().next().getAddress();
        }
        return service;
    }
   

    public String getTftpServer() {
        return m_tftpServer;
    }

    public void setProxyServerAddr(String proxyServerAddr) {
        m_proxyServerAddr = proxyServerAddr;
    }

    /**
     * Only use this function when IP address of the proxy is needed. In most cases you should be
     * able to use SIP domain name
     */
    public String getProxyServerAddr() {
        return m_proxyServerAddr;
    }

    public void setProxyServerSipPort(String proxyServerSipPort) {
        m_proxyServerSipPort = proxyServerSipPort;
    }

    /**
     * Only use this function when port of the the proxy is needed. In most cases you should be
     * able to use SIP domain name
     */
    public String getProxyServerSipPort() {
        return m_proxyServerSipPort;
    }

    /**
     * URL where phone profiles are delivered from apache web server.
     * 
     * @return generated url if not set
     */
    public String getProfileRootUrl() {
        if (m_profileRootUrl != null) {
            return m_profileRootUrl;
        }

        StringBuffer url = new StringBuffer();
        url.append("http://").append(getFullyQualifiedDomainName()).append(":8090");
        url.append("/phone/profile/docroot");

        return url.toString();
    }

    public void setProfileRootUrl(String profileUrl) {
        m_profileRootUrl = profileUrl;
    }

    public void setTftpServer(String tftpServer) {
        m_tftpServer = tftpServer;
    }

    public String getAuthorizationRealm() {
        return m_authorizationRealm;
    }

    public void setAuthorizationRealm(String authorizationRealm) {
        m_authorizationRealm = authorizationRealm;
    }

    public String getVoiceMail() {
        if (m_dialPlanContext == null) {
            return InternalRule.DEFAULT_VOICEMAIL;
        }

        return m_dialPlanContext.getVoiceMail();
    }

    static boolean defaultSipPort(String port) {
        return StringUtils.isBlank(port) || DEFAULT_SIP_PORT.equals(port);
    }

    public String getFullyQualifiedDomainName() {
        return m_fullyQualifiedDomainName;
    }

    public void setFullyQualifiedDomainName(String fullyQualifiedDomainName) {
        m_fullyQualifiedDomainName = fullyQualifiedDomainName;
    }

    public DeviceTimeZone getTimeZone() {
        return m_timeZone;
    }

    public void setDeviceTimeZone(DeviceTimeZone zone) {
        m_timeZone = zone;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public void setSipxServer(SipxServer sipxServer) {
        m_sipxServer = sipxServer;
    }

    public SipxServer getSipxServer() {
        return m_sipxServer;
    }
}
