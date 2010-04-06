/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.nattraversal;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.service.ServiceConfiguratorImpl;
import org.sipfoundry.sipxconfig.service.SipxBridgeService;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRelayService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class NatTraversalManagerImplTestIntegration extends IntegrationTestCase {

    private NatTraversalManager m_natTraversalManager;
    private ServiceConfiguratorImpl m_serviceConfiguratorImpl;
    private SipxProcessContext m_sipxProcessContext;
    private LocationsManager m_locationsManager;
    private SipxRelayService m_sipxRelayService;
    private SipxProxyService m_sipxProxyService;
    private SipxBridgeService m_sipxBridgeService;

    public void setNatTraversalManager(NatTraversalManager natTraversalManager) {
        m_natTraversalManager = natTraversalManager;
    }

    public void setServiceConfiguratorImpl(ServiceConfiguratorImpl serviceConfiguratorImpl) {
        m_serviceConfiguratorImpl = serviceConfiguratorImpl;
    }

    public void setSipxProcessContext(SipxProcessContext sipxProcessContext) {
        m_sipxProcessContext = sipxProcessContext;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setSipxRelayService(SipxRelayService sipxRelayService) {
        m_sipxRelayService = sipxRelayService;
    }

    public void setSipxProxyService(SipxProxyService sipxProxyService) {
        m_sipxProxyService = sipxProxyService;
    }

    public void setSipxBridgeService(SipxBridgeService sipxBridgeService) {
        m_sipxBridgeService = sipxBridgeService;
    }

    /**
     * Tests number of services to restart in various scenarios when NAT Traversal is changed
     * 1.When Internet Calling/Nat Traversal is changed: Relay/Trunking/Proxy services are
     *   marked for restart from all configured locations (if available)
     * 2.When Server/NAT is changed: Relay is marked for restart from all locations (relay settings are changed)
     *   Trunking/Proxy of affected location are marked for restart (if available)
     * @throws Exception
     */
    public void testActivateMarkForRestart() throws Exception {
        m_sipxProcessContext.clear();
        assertEquals(0, m_sipxProcessContext.getRestartNeededServices().size());
        SipxServiceManager serviceManager = TestUtil.getMockSipxServiceManager(
                true, m_sipxRelayService, m_sipxProxyService, m_sipxBridgeService);
        NatTraversal natTraversal = new NatTraversal(serviceManager);
        loadDataSetXml("admin/commserver/seedLocationsAndServices4.xml");
        m_serviceConfiguratorImpl.setLocationsManager(m_locationsManager);
        natTraversal.activate(m_serviceConfiguratorImpl);
        assertEquals(5, m_sipxProcessContext.getRestartNeededServices().size());
        m_sipxProcessContext.clear();
        natTraversal.activateOnLocation(m_locationsManager.getLocation(100), m_serviceConfiguratorImpl);
        assertEquals(4, m_sipxProcessContext.getRestartNeededServices().size());
        m_sipxProcessContext.clear();
        natTraversal.activateOnLocation(m_locationsManager.getLocation(101), m_serviceConfiguratorImpl);
        assertEquals(3, m_sipxProcessContext.getRestartNeededServices().size());
    }

    public void testOneNatTraversalRecord() throws Exception {
        NatTraversal natTraversal = m_natTraversalManager.getNatTraversal();
        assertTrue(natTraversal != null);

        // test default values
        assertTrue(natTraversal.isBehindnat());
        assertTrue(natTraversal.isEnabled());

        // test update NatTraversal
        natTraversal.setEnabled(false);

        m_natTraversalManager.store(natTraversal);

        NatTraversal natTraversalUpdated = m_natTraversalManager.getNatTraversal();
        assertTrue(natTraversalUpdated != null);
        assertFalse(natTraversalUpdated.isEnabled());

        flush();
    }
}
