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

import java.util.ArrayList;
import java.util.Collection;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxBridgeService;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRelayService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

public class NatTraversal {
    private static final String SETTING_ENABLED = "nat/enabled";
    private static final String SETTING_BEHIND = "nat/behind-nat";

    private final SipxRelayService m_relayService;
    private final SipxProxyService m_proxyService;
    private final SipxBridgeService m_bridgeService;
    private boolean m_enabled;
    private boolean m_behindnat;

    public NatTraversal(SipxServiceManager serviceManager) {
        m_relayService = (SipxRelayService) serviceManager.getServiceByBeanId(SipxRelayService.BEAN_ID);
        m_proxyService = (SipxProxyService) serviceManager.getServiceByBeanId(SipxProxyService.BEAN_ID);
        m_bridgeService = (SipxBridgeService) serviceManager.getServiceByBeanId(SipxBridgeService.BEAN_ID);
        m_enabled = (Boolean) m_relayService.getSettingTypedValue(SETTING_ENABLED);
        m_behindnat = (Boolean) m_relayService.getSettingTypedValue(SETTING_BEHIND);
    }

    public void store(SipxServiceManager serviceManager) {
        m_relayService.setSettingTypedValue(SETTING_ENABLED, m_enabled);
        m_relayService.setSettingTypedValue(SETTING_BEHIND, m_behindnat);
        serviceManager.storeService(m_relayService);
    }

    public void activate(ServiceConfigurator serviceConfigurator) {
        serviceConfigurator.replicateServiceConfig(m_relayService);
        Collection<SipxService> services = new ArrayList<SipxService>();
        services.add(m_proxyService);
        services.add(m_bridgeService);
        serviceConfigurator.markServiceForRestart(services);
    }

    public void activateOnLocation(Location location, ServiceConfigurator serviceConfigurator) {
        serviceConfigurator.replicateServiceConfig(location, m_relayService);
        Collection<SipxService> services = new ArrayList<SipxService>();
        services.add(m_proxyService);
        services.add(m_bridgeService);
        serviceConfigurator.markServiceForRestart(location, services);
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
