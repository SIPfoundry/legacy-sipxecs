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
        assertTrue(natTraversal.getSettingValue(NatTraversal.INFO_MAXCONCRELAYS).equals("50"));
        assertFalse(natTraversal.isBehindnat());
        assertFalse(natTraversal.isEnabled());

        //test update NatTraversal
        natTraversal.setEnabled(true);
        natTraversal.setSettingValue(NatTraversal.INFO_MAXCONCRELAYS, "21");
        natTraversal.setSettingValue(NatTraversal.INFO_AGGRESSIVENESS, "Conservative");

        m_natTraversalManager.store(natTraversal);

        NatTraversal natTraversalUpdated = m_natTraversalManager.getNatTraversal();
        assertTrue(natTraversalUpdated != null);
        assertTrue(natTraversalUpdated.getSettingValue(NatTraversal.INFO_MAXCONCRELAYS).equals("21"));
        assertTrue(natTraversalUpdated.getSettingValue(NatTraversal.INFO_AGGRESSIVENESS).equals("Conservative"));
        assertTrue(natTraversalUpdated.isEnabled());

        flush();
    }

}
