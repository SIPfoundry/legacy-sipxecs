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

import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.EmergencyInfo;
import org.sipfoundry.sipxconfig.admin.dialplan.InternalRule;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.ConfiguredService;
import org.sipfoundry.sipxconfig.service.ServiceDescriptor;
import org.sipfoundry.sipxconfig.service.ServiceManager;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.UnmanagedService;
import org.springframework.beans.factory.annotation.Required;

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

    /** see config.defs PROXY_SERVER_ADDR */
    private String m_proxyServerAddr;

    /** see config.defs PROXY_SERVER_SIP_PORT */
    private String m_proxyServerSipPort;

    private DeviceTimeZone m_timeZone = new DeviceTimeZone();

    private ServiceManager m_serviceManager;

    private String m_defaultNtpService = "pool.ntp.org";

    private String m_logDirectory;

    private SipxServiceManager m_sipxServiceManager;

    private String m_mohUser;


    public void setDefaultNtpService(String defaultNtpService) {
        m_defaultNtpService = defaultNtpService;
    }

    public void setServiceManager(ServiceManager serviceManager) {
        m_serviceManager = serviceManager;
    }

    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    public SipxServiceManager getSipxServiceManager() {
        return m_sipxServiceManager;
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
        return getServer(1, UnmanagedService.NTP);
    }

    /**
     * Find IP address (or FQDN) of the specific type of server.
     *
     * @param index 0-based index of the server (0 == Primary, 1 = Secondary, etc._
     * @param s service descriptor
     * @return null if service is not defineds
     */
    public String getServer(int index, ServiceDescriptor s) {
        List<ConfiguredService> servers = m_serviceManager.getEnabledServicesByType(s);
        if (servers == null || servers.size() <= index) {
            return null;
        }
        return servers.get(index).getAddress();
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

    private EmergencyInfo getLikelyEmergencyInfo() {
        if (m_dialPlanContext == null) {
            return null;
        }

        return m_dialPlanContext.getLikelyEmergencyInfo();
    }

    public String getEmergencyAddress() {
        EmergencyInfo info = getLikelyEmergencyInfo();
        return info == null ? null : info.getAddress();
    }

    public Integer getEmergencyPort() {
        EmergencyInfo info = getLikelyEmergencyInfo();
        return info == null ? null : info.getPort();
    }

    public String getEmergencyNumber() {
        EmergencyInfo info = getLikelyEmergencyInfo();
        return info == null ? null : info.getNumber();
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

    @Required
    public void setMohUser(String mohUser) {
        m_mohUser = mohUser;
    }

    public String getMusicOnHoldUri(String domainName) {
        return SipUri.format(m_mohUser, domainName, false);
    }

    public void setLogDirectory(String logDirectory) {
        m_logDirectory = logDirectory;
    }

    public String getLogDirectory() {
        return m_logDirectory;
    }

    public String getDirectedCallPickupCode() {
        SipxRegistrarService registrarService = (SipxRegistrarService) m_sipxServiceManager
                .getServiceByBeanId(SipxRegistrarService.BEAN_ID);
        return registrarService.getDirectedCallPickupCode();
    }

}
