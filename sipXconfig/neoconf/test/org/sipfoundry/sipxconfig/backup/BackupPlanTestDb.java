/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.backup;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.test.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class BackupPlanTestDb extends SipxDatabaseTestCase {

    private BackupManager m_backupManager;

    protected void setUp() throws Exception {
        m_backupManager = (BackupManager) TestHelper.getApplicationContext().getBean(
                BackupManager.CONTEXT_BEAN_NAME);
    }

    public void testStoreJob() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");

        BackupPlan plan = new LocalBackupPlan();
        m_backupManager.storeBackupPlan(plan);

        ITable actual = TestHelper.getConnection().createDataSet().getTable("backup_plan");

        IDataSet expectedDs = TestHelper.loadDataSetFlat("backup/SaveBackupPlanExpected.xml");

        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);

        expectedRds.addReplacementObject("[backup_plan_id]", plan.getId());
        expectedRds.addReplacementObject("[null]", null);
        expectedRds.addReplacementObject("[backup_type]","L");

        ITable expected = expectedRds.getTable("backup_plan");


        Assertion.assertEquals(expected, actual);
    }
}
