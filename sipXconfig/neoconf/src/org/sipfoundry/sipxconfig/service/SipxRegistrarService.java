/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.service;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.commserver.ConflictingFeatureCodeValidator;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel.ProcessName;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxRegistrarService extends SipxService {
    public static final String BEAN_ID = "sipxRegistrarService";

    private static final String USER_CONFIGURED_DOMAIN_ALIASES = "domain/USER_CONFIGURED_DOMAIN_ALIASES";
    private static final String SIP_REGISTRAR_DOMAIN_ALIASES = "domain/SIP_REGISTRAR_DOMAIN_ALIASES";

    private static final ProcessName PROCESS_NAME = ProcessName.REGISTRAR;

    private String m_registrarSipPort;
    private String m_registrarEventSipPort;
    private String m_mediaServerSipSrvOrHostport;
    private String m_orbitServerSipSrvOrHostport;
    private String m_voicemailHttpsPort;
    private String m_proxyServerSipHostport;
    private SipxServiceManager m_sipxServiceManager;

    public String getRegistrarSipPort() {
        return m_registrarSipPort;
    }

    public void setRegistrarSipPort(String sipPort) {
        m_registrarSipPort = sipPort;
    }

    public String getRegistrarEventSipPort() {
        return m_registrarEventSipPort;
    }

    public void setRegistrarEventSipPort(String eventSipPort) {
        m_registrarEventSipPort = eventSipPort;
    }

    public String getMediaServerSipSrvOrHostport() {
        return m_mediaServerSipSrvOrHostport;
    }

    public void setMediaServerSipSrvOrHostport(String serverSipSrvOrHostport) {
        m_mediaServerSipSrvOrHostport = serverSipSrvOrHostport;
    }

    public String getOrbitServerSipSrvOrHostport() {
        return m_orbitServerSipSrvOrHostport;
    }

    public void setOrbitServerSipSrvOrHostport(String serverSipSrvOrHostport) {
        m_orbitServerSipSrvOrHostport = serverSipSrvOrHostport;
    }

    public String getVoicemailHttpsPort() {
        return m_voicemailHttpsPort;
    }

    public void setVoicemailHttpsPort(String httpsPort) {
        m_voicemailHttpsPort = httpsPort;
    }

    public String getProxyServerSipHostport() {
        return m_proxyServerSipHostport;
    }

    public void setProxyServerSipHostport(String serverSipHostport) {
        m_proxyServerSipHostport = serverSipHostport;
    }

    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Override
    public void setFullHostname(String fullHostname) {
        super.setFullHostname(fullHostname);
        if (settingsLoaded()) {
            setRegistrarDomainAliases(null);
        }
    }

    @Override
    public void setIpAddress(String ipAddress) {
        super.setIpAddress(ipAddress);
        if (settingsLoaded()) {
            setRegistrarDomainAliases(null);
        }
    }

    public void setRegistrarDomainAliases(Collection<String> aliases) {
        Setting domainAliasSetting = getSettings().getSetting(SIP_REGISTRAR_DOMAIN_ALIASES);
        Setting userConfiguredAliasSetting = getSettings().getSetting(USER_CONFIGURED_DOMAIN_ALIASES);

        if (aliases != null) {
            userConfiguredAliasSetting.setValue(StringUtils.join(aliases, ' '));
        }

        List<String> allAliases = new ArrayList<String>();
        if (getFullHostname() != null) {
            allAliases.add(getFullHostname());
        }
        if (getIpAddress() != null) {
            allAliases.add(getIpAddress());
        }

        if (userConfiguredAliasSetting.getValue() != null) {
            allAliases.addAll(Arrays.asList(StringUtils.split(userConfiguredAliasSetting.getValue())));
        }
        String aliasesString = StringUtils.join(allAliases.iterator(), ' ');
        domainAliasSetting.setValue(aliasesString);
    }

    /**
     * Validates the data in this service and throws a UserException if there is a problem
     */
    @Override
    public void validate() {
        SipxService presenceService = m_sipxServiceManager.getServiceByBeanId(SipxPresenceService.BEAN_ID);
        new ConflictingFeatureCodeValidator().validate(Arrays.asList(new Setting[] {
            getSettings(), presenceService.getSettings()
        }));
    }

    private boolean settingsLoaded() {
        if (getModelFilesContext() == null || getModelDir() == null || getModelName() == null) {
            return false;
        }
        return getSettings() != null;
    }

    @Override
    public ProcessName getProcessName() {
        return PROCESS_NAME;
    }

    public String getDirectedCallPickupCode() {
        return getSettingValue("call-pick-up/SIP_REDIRECT.180-PICKUP.DIRECTED_CALL_PICKUP_CODE");
    }
}
