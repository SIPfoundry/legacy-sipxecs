/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.gateway;

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class SipTrunkMigrationContextTestDb extends SipxDatabaseTestCase {
    private SipTrunkMigrationContext m_context;

    private GatewayContext m_gatewayContext;

    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_context = (SipTrunkMigrationContext) appContext
                .getBean(SipTrunkMigrationContext.CONTEXT_BEAN_NAME);
        m_gatewayContext = (GatewayContext) appContext.getBean(GatewayContext.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testMigrateSipTrunk() throws Exception {
        TestHelper.insertFlat("gateway/pre_migration_sip_trunk.db.xml");
        assertEquals(1, getConnection().getRowCount("value_storage",
                "where value_storage_id='30'"));
        assertEquals(1, getConnection().getRowCount("setting_value",
                "where value_storage_id='30'"));
        assertEquals(0, getConnection().getRowCount("sbc_device"));
        assertEquals(2, getConnection().getRowCount("gateway"));
        assertEquals(1, getConnection().getRowCount("gateway", "where value_storage_id='30'"));

        m_context.migrateSipTrunk();

        assertEquals(0, getConnection().getRowCount("value_storage"));
        assertEquals(0, getConnection().getRowCount("setting_value"));
        assertEquals(1, getConnection().getRowCount("sbc_device", "where address='9.1.1.1'"));
        assertEquals(1, getConnection().getRowCount("sbc_device"));
        assertEquals(2, getConnection().getRowCount("gateway"));
        assertEquals(0, getConnection().getRowCount("gateway", "where value_storage_id='30'"));

        SipTrunk sipTrunk = (SipTrunk) m_gatewayContext.getGateway(101);
        assertEquals("9.1.1.1", sipTrunk.getRoute());
    }
}
