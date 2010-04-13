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

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;

public class BackupPlanTestDb extends SipxDatabaseTestCase {

    private AdminContext m_adminContext;

    protected void setUp() throws Exception {
        m_adminContext = (AdminContext) TestHelper.getApplicationContext().getBean(
                AdminContext.CONTEXT_BEAN_NAME);
    }

    public void testStoreJob() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");

        BackupPlan plan = new LocalBackupPlan();
        m_adminContext.storeBackupPlan(plan);

        ITable actual = TestHelper.getConnection().createDataSet().getTable("backup_plan");

        IDataSet expectedDs = TestHelper.loadDataSetFlat("admin/SaveBackupPlanExpected.xml");

        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);

        expectedRds.addReplacementObject("[backup_plan_id]", plan.getId());
        expectedRds.addReplacementObject("[null]", null);
        expectedRds.addReplacementObject("[backup_type]","L");

        ITable expected = expectedRds.getTable("backup_plan");


        Assertion.assertEquals(expected, actual);
    }
}
