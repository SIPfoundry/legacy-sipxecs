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

import java.util.Collection;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dialplan.EmergencyInfo;
import org.sipfoundry.sipxconfig.dialplan.InternalRule;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.ftp.FtpManager;
import org.sipfoundry.sipxconfig.moh.MohAddressFactory;
import org.sipfoundry.sipxconfig.moh.MusicOnHoldManager;
import org.sipfoundry.sipxconfig.paging.PagingContext;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.registrar.Registrar;
import org.sipfoundry.sipxconfig.time.NtpManager;
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
    private String m_logDirectory;
    private LocationsManager m_locationsManager;
    private MohAddressFactory m_musicOnHold;
    private PagingContext m_pagingContext;
    private Registrar m_registrar;
    private AddressManager m_addressManager;

    /**
     * If true sipXconfig will attempt to route emergency calls directly through emergency
     * gateways. By default all calls are routed through SIP proxy, which allows for sending
     * alarms and registering CDRs.
     */
    private boolean m_routeEmergencyCallsDirectly;

    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    public void setPagingContext(PagingContext pagingContext) {
        m_pagingContext = pagingContext;
    }

    public String getPagingPrefix() {
        return m_pagingContext.getSettings().getPrefix();
    }

    public String getDomainName() {
        return m_domainManager.getDomain().getName();
    }

    public String getNtpServer() {
        return m_addressManager.getSingleAddress(NtpManager.NTP_SERVER).getAddress();
    }

    /**
     * @return null if not set
     */
    public String getAlternateNtpServer() {
        Collection<Address> ntp = m_addressManager.getAddresses(NtpManager.NTP_SERVER);
        if (ntp.size() > 1) {
            return ((Address) ntp.toArray()[1]).getAddress();
        }
        return null;
    }

    public Address getTftpServer() {
        return m_addressManager.getSingleAddress(FtpManager.TFTP_ADDRESS);
    }

    /**
     * Only use this function when IP address of the proxy is needed. In most cases you should be
     * able to use SIP domain name
     */
    public Address getProxyAddress() {
        return m_addressManager.getSingleAddress(ProxyManager.TCP_ADDRESS);
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
        setMohAddressFactory(musicOnHoldManager.getAddressFactory());
    }

    public void setMohAddressFactory(MohAddressFactory musicOnHold) {
        m_musicOnHold = musicOnHold;
    }

    @Required
    public void setRouteEmergencyCallsDirectly(boolean routeEmergencyCallsDirectly) {
        m_routeEmergencyCallsDirectly = routeEmergencyCallsDirectly;
    }

    public String getMusicOnHoldUri() {
        return m_musicOnHold.getDefaultMohUri();
    }

    public void setLogDirectory(String logDirectory) {
        m_logDirectory = logDirectory;
    }

    public String getLogDirectory() {
        return m_logDirectory;
    }

    public String getDirectedCallPickupCode() {
        return m_registrar.getSettings().getDirectedCallPickupCode();
    }

    public String getCallRetrieveCode() {
        return m_registrar.getSettings().getCallRetrieveCode();
    }

    public void setRegistrar(Registrar registrar) {
        m_registrar = registrar;
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    public DialPlanContext getDialPlanContext() {
        return m_dialPlanContext;
    }

    public DomainManager getDomainManager() {
        return m_domainManager;
    }

    public TimeZoneManager getTimeZoneManager() {
        return m_timeZoneManager;
    }

    public LocationsManager getLocationsManager() {
        return m_locationsManager;
    }

    public PagingContext getPagingContext() {
        return m_pagingContext;
    }

    public AddressManager getAddressManager() {
        return m_addressManager;
    }
}
