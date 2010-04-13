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

import java.util.Collections;

import org.dbunit.Assertion;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.NameInUseException;
import org.sipfoundry.sipxconfig.common.DialPad;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.ValueStorage;
import org.springframework.context.ApplicationContext;

public class AutoAttendantTestDb extends SipxDatabaseTestCase {

    private AutoAttendantManager m_context;

    @Override
    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_context = (AutoAttendantManager) appContext.getBean("autoAttendantManager");
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testUpdate() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/seedAttendant.xml");
        AutoAttendant aa = m_context.getAutoAttendant(new Integer(1000));
        m_context.storeAutoAttendant(aa);
        assertEquals(new Integer(1000), aa.getId());
    }

    public void testSave() throws Exception {
        AutoAttendant aa = m_context.newAutoAttendantWithDefaultGroup();
        aa.setName("test-aa");
        aa.setDescription("aa description");
        aa.setPrompt("thankyou_goodbye.wav");

        AttendantMenu menu = new AttendantMenu();
        menu.addMenuItem(DialPad.NUM_1, AttendantMenuAction.TRANSFER_OUT, "1234");
        menu.addMenuItem(DialPad.NUM_5, AttendantMenuAction.OPERATOR);
        aa.setMenu(menu);

        m_context.storeAutoAttendant(aa);

        // attendant data
        ITable actual = TestHelper.getConnection().createDataSet().getTable("auto_attendant");
        ReplacementDataSet expectedRds = TestHelper
                .loadReplaceableDataSetFlat("admin/dialplan/saveAttendantExpected.xml");
        expectedRds.addReplacementObject("[auto_attendant_id]", aa.getId());
        ITable expected = expectedRds.getTable("auto_attendant");
        Assertion.assertEquals(expected, actual);

        // attendant menu items
        ITable actualItems = TestHelper.getConnection().createDataSet().getTable("attendant_menu_item");
        ITable expectedItems = expectedRds.getTable("attendant_menu_item");
        Assertion.assertEquals(expectedItems, actualItems);
    }

    public void testDefaultAttendantGroup() throws Exception {
        Group defaultGroup = m_context.getDefaultAutoAttendantGroup();
        AutoAttendant aa = m_context.newAutoAttendantWithDefaultGroup();
        Group[] groups = aa.getGroups().toArray(new Group[1]);
        assertEquals(1, groups.length);
        assertEquals(defaultGroup.getPrimaryKey(), groups[0].getPrimaryKey());
    }

    public void testSettings() throws Exception {
        AutoAttendant aa = m_context.newAutoAttendantWithDefaultGroup();
        aa.setName("test-settings");
        aa.setPrompt("thankyou_goodbye.wav");
        aa.setSettingValue("dtmf/interDigitTimeout", "4");
        m_context.storeAutoAttendant(aa);

        // attendant data
        ITable actual = TestHelper.getConnection().createDataSet().getTable("setting_value");
        ReplacementDataSet expectedRds = TestHelper
                .loadReplaceableDataSetFlat("admin/dialplan/saveAttendantSettingsExpected.xml");
        ValueStorage vs = (ValueStorage) aa.getValueStorage();
        expectedRds.addReplacementObject("[value_storage_id]", vs.getId());
        ITable expected = expectedRds.getTable("setting_value");
        Assertion.assertEquals(expected, actual);
    }

    public void testNewAutoAttendantWithDefaultGroup() throws Exception {
        AutoAttendant aa = m_context.newAutoAttendantWithDefaultGroup();
        Group defaultGroup = m_context.getDefaultAutoAttendantGroup();
        m_context.storeAutoAttendant(aa);
        ITable t = TestHelper.getConnection().createDataSet().getTable("attendant_group");
        assertEquals(defaultGroup.getId(), t.getValue(0, "group_id"));
    }

    public void testDelete() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/seedAttendant.xml");
        AutoAttendant aa = m_context.getAutoAttendant(new Integer(1000));
        m_context.deleteAutoAttendant(aa);
        ITable actualItems = TestHelper.getConnection().createDataSet().getTable("attendant_menu_item");
        assertEquals(0, actualItems.getRowCount());
    }

    public void testDeleteInUseByAttendantRule() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/attendant_rule.db.xml");
        try {
            m_context.deleteAutoAttendantsByIds(Collections.singletonList(1001));
            fail();
        } catch (AttendantInUseException e) {
            assertTrue(true);
            assertTrue(e.getMessage().indexOf("attendant_rule") > 0);
        }
    }

    public void testDeleteOperatorInUse() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/seedOperator.xml");
        AutoAttendant aa = m_context.getAutoAttendant(new Integer(1000));
        try {
            m_context.deleteAutoAttendant(aa);
            fail();
        } catch (AttendantInUseException e) {
            assertTrue(true);
        }
        aa = m_context.getAutoAttendant(new Integer(1001));
        try {
            m_context.deleteAutoAttendant(aa);
            fail();
        } catch (AttendantInUseException e) {
            assertTrue(true);
        }
    }

    public void testSaveNameThatIsDuplicateAlias() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/seedUser.xml");
        boolean gotNameInUseException = false;
        AutoAttendant aa = m_context.newAutoAttendantWithDefaultGroup();
        aa.setName("alpha");
        try {
            m_context.storeAutoAttendant(aa);
        } catch (NameInUseException e) {
            gotNameInUseException = true;
        }
        assertTrue(gotNameInUseException);
    }

    public void testGetAutoAttendantSettings() throws Exception {
        TestHelper.cleanInsert("admin/dialplan/seedDialPlanWithAttendant.xml");
        AutoAttendant autoAttendant = m_context.getAutoAttendant(new Integer(2000));
        assertNotNull(autoAttendant.getSettings());
    }

    public void testSelectSpecial() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/seedOperator.xml");
        AutoAttendant operator = m_context.getOperator();
        m_context.selectSpecial(operator);

        ITable actualItems = TestHelper.getConnection().createDataSet().getTable("attendant_special_mode");
        assertEquals(1, actualItems.getRowCount());
        assertEquals(1000, actualItems.getValue(0, "auto_attendant_id"));
    }

    public void testDeselectSpecial() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/seedOperator.xml");
        AutoAttendant operator = m_context.getOperator();
        m_context.deselectSpecial(operator);
        ITable actualItems = TestHelper.getConnection().createDataSet().getTable("attendant_special_mode");
        assertEquals(1, actualItems.getRowCount());
        assertEquals(null, actualItems.getValue(0, "auto_attendant_id"));
    }

    public void testGetSetSpecialMode() throws Exception {
        m_context.setSpecialMode(false);
        ITable actualItems = TestHelper.getConnection().createDataSet().getTable("attendant_special_mode");
        assertEquals(1, actualItems.getRowCount());
        assertEquals(false, actualItems.getValue(0, "enabled"));
        m_context.setSpecialMode(true);
        actualItems = TestHelper.getConnection().createDataSet().getTable("attendant_special_mode");
        assertEquals(1, actualItems.getRowCount());
        assertEquals(true, actualItems.getValue(0, "enabled"));
    }

    public void testGetSelectedSpecialAttendant() throws Exception {
        TestHelper.cleanInsertFlat("admin/dialplan/seedSpecialSelectedAttendant.xml");
        AutoAttendant operator = m_context.getOperator();
        AutoAttendant specialAa = m_context.getSelectedSpecialAttendant();
        assertEquals(operator, specialAa);
    }
}
