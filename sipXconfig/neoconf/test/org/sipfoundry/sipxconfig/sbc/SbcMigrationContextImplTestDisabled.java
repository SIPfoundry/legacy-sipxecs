/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.sbc;

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class SbcMigrationContextImplTestDisabled extends IntegrationTestCase {
    private SbcMigrationContext m_sbcMigrationContext;
    private SbcManager m_sbcManager;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    public void testMigrateSbc() throws Exception {
        TestHelper.cleanInsertFlat("sbc/pre_migration_sbc_address.db.xml");
        assertEquals(0, getConnection().getRowCount("sbc_device"));
        assertEquals(3, getConnection().getRowCount("sbc"));
        Sbc sbc = m_sbcManager.loadDefaultSbc();
        assertNull(sbc.getSbcDevice());
        AuxSbc auxSbc = m_sbcManager.loadSbc(1001);
        assertNull(auxSbc.getSbcDevice());

        m_sbcMigrationContext.migrateSbc();

        assertEquals(3, getConnection().getRowCount("sbc_device"));
        assertEquals(3, getConnection().getRowCount("sbc"));
        sbc = m_sbcManager.loadDefaultSbc();
        assertNotNull(sbc.getSbcDevice());
        assertEquals("10.1.2.3", sbc.getSbcDevice().getAddress());
        auxSbc = m_sbcManager.loadSbc(1001);
        assertNotNull(auxSbc.getSbcDevice());
        assertEquals("10.1.2.4", auxSbc.getSbcDevice().getAddress());

        m_sbcManager.clear();
    }

    public void setSbcMigrationContext(SbcMigrationContext sbcMigrationContext) {
        m_sbcMigrationContext = sbcMigrationContext;
    }

    public void setSbcManager(SbcManager sbcManager) {
        m_sbcManager = sbcManager;
    }
}
