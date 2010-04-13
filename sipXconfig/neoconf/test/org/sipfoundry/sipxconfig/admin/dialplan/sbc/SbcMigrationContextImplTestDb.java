/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan.sbc;

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class SbcMigrationContextImplTestDb extends SipxDatabaseTestCase {
    private SbcMigrationContext m_context;

    private SbcManager m_sbcManager;

    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_context = (SbcMigrationContext) appContext
                .getBean(SbcMigrationContext.CONTEXT_BEAN_NAME);
        m_sbcManager = (SbcManager) appContext.getBean(SbcManager.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testMigrateSbc() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/sbc/pre_migration_sbc_address.db.xml");
        assertEquals(0, getConnection().getRowCount("sbc_device"));
        assertEquals(3, getConnection().getRowCount("sbc"));
        Sbc sbc = m_sbcManager.loadDefaultSbc();
        assertNull(sbc.getSbcDevice());
        AuxSbc auxSbc = m_sbcManager.loadSbc(1001);
        assertNull(auxSbc.getSbcDevice());

        m_context.migrateSbc();

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
}
