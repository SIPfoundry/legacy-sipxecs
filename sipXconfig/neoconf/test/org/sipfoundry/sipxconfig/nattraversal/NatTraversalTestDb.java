package org.sipfoundry.sipxconfig.nattraversal;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.ValueStorage;

public class NatTraversalTestDb extends SipxDatabaseTestCase {

    private NatTraversalManager m_natTraversalManager;

    protected void setUp() throws Exception {
        m_natTraversalManager = (NatTraversalManager) TestHelper.getApplicationContext().getBean(
                NatTraversalManager.CONTEXT_BEAN_NAME);
    }

    public void testStoreJob() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");

        NatTraversal natTraversal = (NatTraversal) TestHelper.getApplicationContext().
                                        getBean("natTraversal");
        natTraversal.setEnabled(true);
        natTraversal.setBehindnat(false);
        natTraversal.getSettings().getSetting("nattraversal-info/concurrentrelays").setValue("2");
        ValueStorage valueStorage = (ValueStorage)natTraversal.getValueStorage();

        m_natTraversalManager.store(natTraversal);

        ITable actualNatTraversal = TestHelper.getConnection().createDataSet().getTable(
                "nat_traversal");
        IDataSet expectedDsNat = TestHelper
                .loadDataSetFlat("nattraversal/NatTraversalExpected.xml");

        ReplacementDataSet expectedRdsNat = new ReplacementDataSet(expectedDsNat);

        expectedRdsNat.addReplacementObject("[id]", natTraversal.getId());
        expectedRdsNat.addReplacementObject("[enabled]", natTraversal.isEnabled());
        expectedRdsNat.addReplacementObject("[behindnat]", natTraversal.isBehindnat());
        expectedRdsNat.addReplacementObject("[value_storage_id]", valueStorage.getId());

        ITable expectedConf = expectedRdsNat.getTable("nat_traversal");

        Assertion.assertEquals(expectedConf, actualNatTraversal);
        assertEquals("2", natTraversal.getSettings().
                getSetting("nattraversal-info/concurrentrelays").getValue());
    }
}
