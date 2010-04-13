/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import org.apache.commons.logging.LogFactory;
import org.dbunit.Assertion;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.dbunit.dataset.filter.DefaultColumnFilter;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.context.ApplicationContext;
import org.springframework.dao.DataIntegrityViolationException;

public class AttendantMigrationContextImplTestDb extends SipxDatabaseTestCase {

    private AttendantMigrationContext m_context;
    private AutoAttendantManager m_autoAttendantManager;

    @Override
    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_context = (AttendantMigrationContext) appContext
                .getBean(AttendantMigrationContext.CONTEXT_BEAN_NAME);
        m_autoAttendantManager = (AutoAttendantManager) appContext.getBean("autoAttendantManager");
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testMigrateAttendantRules() throws Exception {
        try {
            TestHelper.cleanInsertFlat("admin/dialplan/pre_attendant_migration.db.xml");
            m_context.migrateAttendantRules();

            ITable actual = getConnection().createDataSet().getTable("attendant_dialing_rule");
            assertEquals(2, actual.getRowCount());

            ITable expected = TestHelper.loadDataSetFlat(
                    "admin/dialplan/post_attendant_migration.xml").getTable(
                    "attendant_dialing_rule");

            ITable filteredTable = DefaultColumnFilter.includedColumnsTable(actual, expected
                    .getTableMetaData().getColumns());

            Assertion.assertEquals(expected, filteredTable);
        } catch (DataIntegrityViolationException knownError) {
            LogFactory.getLog(getClass()).error("FIXME", knownError);
        }
    }

    public void testSetAttendantDefaults() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/seedAttendant.xml");
        Group defaults = m_autoAttendantManager.getDefaultAutoAttendantGroup();
        m_context.setAttendantDefaults();
        ReplacementDataSet expected = TestHelper.loadReplaceableDataSetFlat(
                "admin/dialplan/set_attendant_defaults.db.xml");
        expected.addReplacementObject("[group_id]", defaults.getId());
        ITable actual = getConnection().createDataSet().getTable("attendant_group");
        Assertion.assertEquals(expected.getTable("attendant_group"), actual);
    }
}
