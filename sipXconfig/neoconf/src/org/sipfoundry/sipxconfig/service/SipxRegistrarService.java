/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.service;

import java.util.Arrays;

import org.sipfoundry.sipxconfig.admin.commserver.ConflictingFeatureCodeValidator;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxRegistrarService extends SipxService {

    public static final String BEAN_ID = "sipxRegistrarService";

    private String m_registrarEventSipPort;
    private String m_orbitServerSipSrvOrHostport;
    private String m_proxyServerSipHostport;
    private SipxServiceManager m_sipxServiceManager;

    public String getRegistrarEventSipPort() {
        return m_registrarEventSipPort;
    }

    public void setRegistrarEventSipPort(String eventSipPort) {
        m_registrarEventSipPort = eventSipPort;
    }

    public String getOrbitServerSipSrvOrHostport() {
        return m_orbitServerSipSrvOrHostport;
    }

    public void setOrbitServerSipSrvOrHostport(String serverSipSrvOrHostport) {
        m_orbitServerSipSrvOrHostport = serverSipSrvOrHostport;
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

    public String getDirectedCallPickupCode() {
        return getSettingValue("call-pick-up/SIP_REDIRECT.100-PICKUP.DIRECTED_CALL_PICKUP_CODE");
    }
}
