package org.sipfoundry.sipxconfig.nattraversal;

import org.sipfoundry.sipxconfig.IntegrationTestCase;

public class NatTraversalManagerImplTestIntegration extends IntegrationTestCase {
    private NatTraversalManager m_natTraversalManager;

    public void setNatTraversalManager(NatTraversalManager natTraversalManager) {
        m_natTraversalManager = natTraversalManager;
    }

    public void testOneNatTraversalRecord() throws Exception {
        loadDataSetXml("nattraversal/clearNatTraversal.xml");

        //no matter how many times we execute the init task there should be one single Nat Traversal record
        m_natTraversalManager.saveDefaultNatTraversal();
        m_natTraversalManager.saveDefaultNatTraversal();
        NatTraversal natTraversal = m_natTraversalManager.getNatTraversal();
        assertTrue(natTraversal != null);

        //test default values
        assertTrue(natTraversal.getSettingValue("nattraversal-info/concurrentrelays").equals("50"));
        assertFalse(natTraversal.isBehindnat());
        assertFalse(natTraversal.isEnabled());

        //test update NatTraversal
        natTraversal.setEnabled(true);
        natTraversal.setSettingValue("nattraversal-info/concurrentrelays", "21");
        natTraversal.setSettingValue("nattraversal-info/relayaggressiveness", "Conservative");

        m_natTraversalManager.store(natTraversal);

        NatTraversal natTraversalUpdated = m_natTraversalManager.getNatTraversal();
        assertTrue(natTraversalUpdated != null);
        assertTrue(natTraversalUpdated.getSettingValue("nattraversal-info/concurrentrelays").equals("21"));
        assertTrue(natTraversalUpdated.getSettingValue("nattraversal-info/relayaggressiveness").equals("Conservative"));
        assertTrue(natTraversalUpdated.isEnabled());

        flush();
    }

}
