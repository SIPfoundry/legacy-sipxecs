package org.sipfoundry.sipxconfig.nattraversal;

import org.sipfoundry.sipxconfig.IntegrationTestCase;

public class NatTraversalManagerImplTestIntegration extends IntegrationTestCase {

    private NatTraversalManager m_natTraversalManager;

    public void setNatTraversalManager(NatTraversalManager natTraversalManager) {
        m_natTraversalManager = natTraversalManager;
    }

    public void testOneNatTraversalRecord() throws Exception {
        NatTraversal natTraversal = m_natTraversalManager.getNatTraversal();
        assertTrue(natTraversal != null);

        // test default values
        assertFalse(natTraversal.isBehindnat());
        assertFalse(natTraversal.isEnabled());

        // test update NatTraversal
        natTraversal.setEnabled(true);

        m_natTraversalManager.store(natTraversal);

        NatTraversal natTraversalUpdated = m_natTraversalManager.getNatTraversal();
        assertTrue(natTraversalUpdated != null);
        assertTrue(natTraversalUpdated.isEnabled());

        flush();
    }
}
