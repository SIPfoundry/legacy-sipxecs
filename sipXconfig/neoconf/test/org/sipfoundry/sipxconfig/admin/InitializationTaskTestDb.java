/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import org.dbunit.dataset.ITable;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;

public class InitializationTaskTestDb extends SipxDatabaseTestCase {

    private AdminContext m_adminContext;

    protected void setUp() throws Exception {
        m_adminContext = (AdminContext) TestHelper.getApplicationContext().getBean(
                AdminContext.CONTEXT_BEAN_NAME);
    }

    public void testDeleteInitializationTask() throws Exception {
        TestHelper.cleanInsertFlat("admin/InitializationTaskSeed.xml");

        m_adminContext.deleteInitializationTask("test-task");

        ITable actual = TestHelper.getConnection().createDataSet().getTable("initialization_task");
        assertEquals(0, actual.getRowCount());
    }

    public void testGetInitializationTask() throws Exception {
        TestHelper.cleanInsertFlat("admin/InitializationTaskSeed.xml");

        String[] tasks = m_adminContext.getInitializationTasks();
        assertEquals(1, tasks.length);
        assertEquals("test-task", tasks[0]);
    }
}
