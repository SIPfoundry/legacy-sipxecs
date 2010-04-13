/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd;

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.InitializationTask;
import org.springframework.context.ApplicationContext;

public class AcdMigrationTriggerTestDb extends SipxDatabaseTestCase {
    private ApplicationContext m_applicationContext;
    private AcdContext m_context;

    @Override
    protected void setUp() throws Exception {
        m_applicationContext = TestHelper.getApplicationContext();
        m_context = (AcdContext) TestHelper.getApplicationContext().getBean(AcdContext.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testMigrateLineExtensions() throws Exception {
        TestHelper.insertFlat("acd/migrate_lines.db.xml");

        assertEquals(2, getConnection().getRowCount("setting_value"));
        assertEquals(0, getConnection().getRowCount("acd_line", "where extension = '2222'"));
        assertEquals(0, getConnection().getRowCount("acd_line", "where extension = '1111'"));

        InitializationTask task = new InitializationTask("acd_migrate_line_extensions");

        m_applicationContext.publishEvent(task);

        assertEquals(0, getConnection().getRowCount("setting_value"));
        assertEquals(1, getConnection().getRowCount("acd_line", "where extension = '2222'"));
        assertEquals(1, getConnection().getRowCount("acd_line", "where extension = '1111'"));
    }

    public void testMigrateOverflowQueues() throws Exception {
        TestHelper.insertFlat("acd/migrate_queues.db.xml");

        assertEquals(1, getConnection().getRowCount("setting_value"));
        assertEquals(0, getConnection().getRowCount("acd_queue",
                "where acd_queue_id = 2001 AND overflow_queue_id = 2002"));
        InitializationTask task = new InitializationTask("acd_migrate_overflow_queues");
        m_applicationContext.publishEvent(task);

        assertEquals(1, getConnection().getRowCount("acd_queue",
                "where acd_queue_id = 2001 AND overflow_queue_id = 2002"));
        assertEquals(0, getConnection().getRowCount("setting_value"));
    }

    public void testMigrateAcdServers() throws Exception {
        TestHelper.insertFlat("acd/migrate_acd_servers.db.xml");

        // test below rely on the fact that 14 is service ID of ACD service
        assertEquals(1, getConnection().getRowCount("sipx_service",
                "where bean_id='sipxAcdService' AND sipx_service_id=13"));

        assertEquals(3, getConnection().getRowCount("acd_server"));
        assertEquals(1, getConnection().getRowCount("location"));
        assertEquals(1, getConnection().getRowCount("location_specific_service", "where sipx_service_id=13"));

        InitializationTask task = new InitializationTask("acd_server_migrate_acd_service");
        m_applicationContext.publishEvent(task);

        assertEquals(3, getConnection().getRowCount("acd_server"));
        assertEquals(3, getConnection().getRowCount("location"));
        assertEquals(3, getConnection().getRowCount("location_specific_service", "where sipx_service_id=13"));

        assertEquals(1, getConnection().getRowCount("acd_server", "where location_id=1001"));

        m_context.clear();
    }
}
