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

import java.util.Date;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;

public class DailyBackupScheduleTestDb extends SipxDatabaseTestCase {
    
    private AdminContext m_adminContext;
    
    protected void setUp() throws Exception {
        m_adminContext = (AdminContext) TestHelper.getApplicationContext().getBean(
                AdminContext.CONTEXT_BEAN_NAME);
    }
    
    public void testStoreJob() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        
        BackupPlan plan = m_adminContext.getBackupPlan();
        DailyBackupSchedule dailySchedule = new DailyBackupSchedule();
        plan.addSchedule(dailySchedule);
        m_adminContext.storeBackupPlan(plan);
        
        ITable actual = TestHelper.getConnection().createDataSet().getTable("daily_backup_schedule");

        IDataSet expectedDs = TestHelper.loadDataSetFlat("admin/SaveDailyBackupScheduleExpected.xml"); 
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[backup_plan_id]", plan.getId());
        expectedRds.addReplacementObject("[daily_backup_schedule_id]", dailySchedule.getId());
        expectedRds.addReplacementObject("[time_of_day]", new Date(0));        
        expectedRds.addReplacementObject("[null]", null);
        
        ITable expected = expectedRds.getTable("daily_backup_schedule");
                
        Assertion.assertEquals(expected, actual);        
    }

}
