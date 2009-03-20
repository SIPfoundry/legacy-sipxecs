/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.nattraversal;

import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxRelayService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

public class NatTraversal {
    private static final String SETTING_BEHIND = "nat/enabled";
    private static final String SETTING_ENABLED = "nat/behind-nat";

    private final SipxRelayService m_service;
    private boolean m_enabled;
    private boolean m_behindnat;

    public NatTraversal(SipxServiceManager serviceManager) {
        m_service = (SipxRelayService) serviceManager.getServiceByBeanId(SipxRelayService.BEAN_ID);
        m_enabled = (Boolean) m_service.getSettingTypedValue(SETTING_ENABLED);
        m_behindnat = (Boolean) m_service.getSettingTypedValue(SETTING_BEHIND);
    }

    public void store(SipxServiceManager serviceManager) {
        m_service.setSettingTypedValue(SETTING_ENABLED, m_enabled);
        m_service.setSettingTypedValue(SETTING_BEHIND, m_behindnat);
        serviceManager.storeService(m_service);
    }

    public void activate(ServiceConfigurator serviceConfigurator) {
        serviceConfigurator.replicateServiceConfig(m_service);
    }

    public boolean isBehindnat() {
        return m_behindnat;
    }

    public void setBehindnat(boolean behindnat) {
        m_behindnat = behindnat;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }
}
