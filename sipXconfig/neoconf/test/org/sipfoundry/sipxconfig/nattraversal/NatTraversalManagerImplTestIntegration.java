package org.sipfoundry.sipxconfig.nattraversal;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.common.UserException;

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

    public void testCheckForRTPPortRangeOverlap() {
        m_natTraversalManager.saveDefaultNatTraversal();
        //there is no sbc bridge
        NatTraversal natTraversal = m_natTraversalManager.getNatTraversal();
        //test start port must be lower than end port
        natTraversal.setSettingValue(NatTraversal.RTP_PORT_START, "100");
        natTraversal.setSettingValue(NatTraversal.RTP_PORT_END, "80");
        try {
            m_natTraversalManager.store(natTraversal);
            fail("Start port should be lower than end port");
        } catch (UserException e) {
            //ok start port is lower than end port
        }
        //test start port is lower than end port
        natTraversal.setSettingValue(NatTraversal.RTP_PORT_START, "100");
        natTraversal.setSettingValue(NatTraversal.RTP_PORT_END, "200");
        try {
            m_natTraversalManager.store(natTraversal);
            //ok Start port is lower than end port
        } catch (UserException e) {
            fail("Start port is lower than end port - there should be no exception");
        }
    }

}
