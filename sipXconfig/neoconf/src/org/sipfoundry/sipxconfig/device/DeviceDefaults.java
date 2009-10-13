/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.device;

import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.EmergencyInfo;
import org.sipfoundry.sipxconfig.admin.dialplan.InternalRule;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.moh.MusicOnHoldManager;
import org.sipfoundry.sipxconfig.paging.PagingContext;
import org.sipfoundry.sipxconfig.service.ConfiguredService;
import org.sipfoundry.sipxconfig.service.ServiceDescriptor;
import org.sipfoundry.sipxconfig.service.ServiceManager;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.UnmanagedService;
import org.springframework.beans.factory.annotation.Required;

/**
 * Sets up phone and line objects with system defaults.
 */
public class DeviceDefaults {
    private static final Log LOG = LogFactory.getLog(DeviceDefaults.class);

    private static final String DEFAULT_SIP_PORT = "5060";

    private String m_profileRootUrl;

    private DialPlanContext m_dialPlanContext;

    private DomainManager m_domainManager;

    private TimeZoneManager m_timeZoneManager;

    private ServiceManager m_serviceManager;

    private String m_defaultNtpService = "pool.ntp.org";

    private String m_logDirectory;

    private SipxServiceManager m_sipxServiceManager;

    private LocationsManager m_locationsManager;

    private MusicOnHoldManager m_musicOnHoldManager;

    private PagingContext m_pagingContext;

    /**
     * If true sipXconfig will attempt to route emergency calls directly through emergency
     * gateways. By default all calls are routed through SIP proxy, which allows for sending
     * alarms and registering CDRs.
     */
    private boolean m_routeEmergencyCallsDirectly;

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

    public void setPagingContext(PagingContext pagingContext) {
        m_pagingContext = pagingContext;
    }

    public String getPagingPrefix() {
        return m_pagingContext.getPagingPrefix();
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
     * @return null if service is not defined
     */
    public String getServer(int index, ServiceDescriptor s) {
        List<ConfiguredService> servers = m_serviceManager.getEnabledServicesByType(s);
        if (servers == null || servers.size() <= index) {
            return null;
        }
        return servers.get(index).getAddress();
    }

    public String getTftpServer() {
        return m_locationsManager.getPrimaryLocation().getAddress();
    }

    /**
     * Only use this function when IP address of the proxy is needed. In most cases you should be
     * able to use SIP domain name
     */
    public String getProxyServerAddr() {
        return m_locationsManager.getPrimaryLocation().getAddress();
    }

    /**
     * Only use this function when port of the the proxy is needed. In most cases you should be
     * able to use SIP domain name
     */
    public String getProxyServerSipPort() {
        SipxProxyService sipxProxyService = (SipxProxyService) m_sipxServiceManager
                .getServiceByBeanId(SipxProxyService.BEAN_ID);
        return sipxProxyService.getSipPort();
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

    public String getAuthorizationRealm() {
        String authorizationRealm = null;
        try {
            authorizationRealm = m_domainManager.getDomain().getSipRealm();
        } catch (DomainManager.DomainNotInitializedException e) {
            LOG.warn("Unable to get authorization realm; domain not initiazlized", e);
        }

        return authorizationRealm;
    }

    public String getVoiceMail() {
        if (m_dialPlanContext == null) {
            return InternalRule.DEFAULT_VOICEMAIL;
        }

        return m_dialPlanContext.getVoiceMail();
    }

    private EmergencyInfo getLikelyEmergencyInfo() {
        if (!m_routeEmergencyCallsDirectly) {
            return null;
        }

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

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public String getFullyQualifiedDomainName() {
        return m_locationsManager.getPrimaryLocation().getFqdn();
    }

    public void setTimeZoneManager(TimeZoneManager tzm) {
        m_timeZoneManager = tzm;
    }

    public DeviceTimeZone getTimeZone() {
        return m_timeZoneManager.getDeviceTimeZone();
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    @Required
    public void setMusicOnHoldManager(MusicOnHoldManager musicOnHoldManager) {
        m_musicOnHoldManager = musicOnHoldManager;
    }

    @Required
    public void setRouteEmergencyCallsDirectly(boolean routeEmergencyCallsDirectly) {
        m_routeEmergencyCallsDirectly = routeEmergencyCallsDirectly;
    }

    public String getMusicOnHoldUri() {
        return m_musicOnHoldManager.getDefaultMohUri();
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

    public String getCallRetrieveCode() {
        SipxRegistrarService registrarService = (SipxRegistrarService) m_sipxServiceManager
                .getServiceByBeanId(SipxRegistrarService.BEAN_ID);
        return registrarService.getCallRetrieveCode();
    }

}
