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

    private static final String VOICEMAIL_SERVER = "https://localhost";

    private String m_registrarEventSipPort;
    private String m_mediaServerSipSrvOrHostport;
    private String m_orbitServerSipSrvOrHostport;
    private String m_proxyServerSipHostport;
    private SipxServiceManager m_sipxServiceManager;

    public String getMediaServer() {
        return m_mediaServerSipSrvOrHostport + ";transport=tcp";
    }

    public String getVoicemailServer() {
        StringBuffer voicemailServer = new StringBuffer();
        voicemailServer.append(VOICEMAIL_SERVER);
        if (getVoicemailHttpsPort() != null) {
            voicemailServer.append(':');
            voicemailServer.append(getVoicemailHttpsPort());
        }

        return voicemailServer.toString();
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

    @Override
    public String getBeanId() {
        return BEAN_ID;
    }
}
