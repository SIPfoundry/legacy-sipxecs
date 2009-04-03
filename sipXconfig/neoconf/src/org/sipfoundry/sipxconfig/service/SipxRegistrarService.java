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
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxRegistrarService extends SipxService implements LoggingEntity {
    public static final String BEAN_ID = "sipxRegistrarService";

    public static final String LOG_SETTING = "logging/SIP_REGISTRAR_LOG_LEVEL";

    private String m_registrarEventSipPort;
    private String m_proxyServerSipHostport;

    public String getRegistrarEventSipPort() {
        return m_registrarEventSipPort;
    }

    public void setRegistrarEventSipPort(String eventSipPort) {
        m_registrarEventSipPort = eventSipPort;
    }

    public String getProxyServerSipHostport() {
        return m_proxyServerSipHostport;
    }

    public void setProxyServerSipHostport(String serverSipHostport) {
        m_proxyServerSipHostport = serverSipHostport;
    }

    /**
     * Validates the data in this service and throws a UserException if there is a problem
     */
    @Override
    public void validate() {
        SipxService presenceService = getSipxServiceManager().getServiceByBeanId(SipxPresenceService.BEAN_ID);
        new ConflictingFeatureCodeValidator().validate(Arrays.asList(new Setting[] {
            getSettings(), presenceService.getSettings()
        }));
    }

    public String getDirectedCallPickupCode() {
        return getSettingValue("call-pick-up/SIP_REDIRECT.100-PICKUP.DIRECTED_CALL_PICKUP_CODE");
    }

    public String getCallRetrieveCode() {
        return getSettingValue("call-pick-up/SIP_REDIRECT.100-PICKUP.CALL_RETRIEVE_CODE");
    }

    @Override
    public String getLogSetting() {
        return LOG_SETTING;
    }

    @Override
    public void setLogLevel(String logLevel) {
        super.setLogLevel(logLevel);
    }

    @Override
    public String getLogLevel() {
        return super.getLogLevel();
    }

    @Override
    public String getLabelKey() {
        return super.getLabelKey();
    }

    /**
     * Generates part of SIP URI used to forward traffic to registrar
     */
    public String formatMappingLocation(Location location, boolean multipleRegistrars) {
        String fqdn = location.getFqdn();
        if (multipleRegistrars) {
            // more than one service - use DNS SRV
            return "rr." + fqdn;
        }
        return String.format("%s:%s;transport=tcp", fqdn, getSipPort());
    }
}
