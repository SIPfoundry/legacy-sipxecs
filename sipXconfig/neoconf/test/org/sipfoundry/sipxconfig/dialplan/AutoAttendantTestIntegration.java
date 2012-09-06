/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dialplan;


import static org.junit.Assert.assertArrayEquals;

import java.util.Collections;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.DialPad;
import org.sipfoundry.sipxconfig.common.NameInUseException;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.ResultDataGrid;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class AutoAttendantTestIntegration extends IntegrationTestCase {
    private AutoAttendantManager m_autoAttendantManager;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        sql("dialplan/attendant/SeedAttendantSpecialMode.sql");
    }

    public void testUpdate() throws Exception {
        loadDataSet("dialplan/seedAttendant.xml");
        AutoAttendant aa = m_autoAttendantManager.getAutoAttendant(new Integer(1000));
        m_autoAttendantManager.storeAutoAttendant(aa);
        assertEquals(new Integer(1000), aa.getId());
    }

    public void testSave() throws Exception {
        AutoAttendant aa = m_autoAttendantManager.newAutoAttendantWithDefaultGroup();
        aa.setName("test-aa");
        aa.setDescription("aa description");
        aa.setPrompt("thankyou_goodbye.wav");

        AttendantMenu menu = new AttendantMenu();
        menu.addMenuItem(DialPad.NUM_1, AttendantMenuAction.TRANSFER_OUT, "1234");
        menu.addMenuItem(DialPad.NUM_5, AttendantMenuAction.OPERATOR);
        aa.setMenu(menu);

        m_autoAttendantManager.storeAutoAttendant(aa);
        flush();
        
        Object[][] expected = new Object[][] {
                {"test-aa", "1", "transfer_out", "1234"},
                {"test-aa", "5", "operator", null}
        };
        ResultDataGrid actual = new ResultDataGrid();
        db().query("select name, dialpad_key, action, parameter from auto_attendant as a, " +
                "attendant_menu_item as m where a.auto_attendant_id = m.auto_attendant_id order by dialpad_key",
                actual);
        assertArrayEquals(expected, actual.toArray());
    }

    public void testDefaultAttendantGroup() throws Exception {
        Group defaultGroup = m_autoAttendantManager.getDefaultAutoAttendantGroup();
        AutoAttendant aa = m_autoAttendantManager.newAutoAttendantWithDefaultGroup();
        Group[] groups = aa.getGroups().toArray(new Group[1]);
        assertEquals(1, groups.length);
        assertEquals(defaultGroup.getPrimaryKey(), groups[0].getPrimaryKey());
    }

    public void testSettings() throws Exception {
        AutoAttendant aa = m_autoAttendantManager.newAutoAttendantWithDefaultGroup();
        aa.setName("test-settings");
        aa.setPrompt("thankyou_goodbye.wav");
        aa.setSettingValue("dtmf/interDigitTimeout", "4");
        m_autoAttendantManager.storeAutoAttendant(aa);
        
        Map<String, Object> actual = db().queryForMap("select * from setting_value");
        assertEquals("dtmf/interDigitTimeout", actual.get("path"));
        assertEquals("4", actual.get("value"));
    }

    public void testNewAutoAttendantWithDefaultGroup() throws Exception {
        AutoAttendant aa = m_autoAttendantManager.newAutoAttendantWithDefaultGroup();
        Group defaultGroup = m_autoAttendantManager.getDefaultAutoAttendantGroup();
        m_autoAttendantManager.storeAutoAttendant(aa);
        Map<String, Object> actual = db().queryForMap("select * from attendant_group");
        assertEquals(defaultGroup.getId(), actual.get("group_id"));
    }

    public void testDelete() throws Exception {
        loadDataSet("dialplan/seedAttendant.xml");
        AutoAttendant aa = m_autoAttendantManager.getAutoAttendant(new Integer(1000));
        m_autoAttendantManager.deleteAutoAttendant(aa);
        assertEquals(0, countRowsInTable("attendant_menu_item"));
    }

    public void testDeleteInUseByAttendantRule() throws Exception {
        loadDataSet("dialplan/attendant_rule.db.xml");
        try {
            m_autoAttendantManager.deleteAutoAttendantsByIds(Collections.singletonList(1001));
            fail();
        } catch (AttendantInUseException e) {
            assertTrue(true);
            assertEquals(e.getMessage(),"&error.attendantInUse");
        }
    }

    public void testDeleteOperatorInUse() throws Exception {
        loadDataSet("dialplan/seedOperator.xml");
        AutoAttendant aa = m_autoAttendantManager.getAutoAttendant(new Integer(1000));
        try {
            m_autoAttendantManager.deleteAutoAttendant(aa);
            fail();
        } catch (AttendantInUseException e) {
            assertTrue(true);
        }
        aa = m_autoAttendantManager.getAutoAttendant(new Integer(1001));
        try {
            m_autoAttendantManager.deleteAutoAttendant(aa);
            fail();
        } catch (AttendantInUseException e) {
            assertTrue(true);
        }
    }

    public void testSaveNameThatIsDuplicateAlias() throws Exception {
        loadDataSet("dialplan/seedUser.xml");
        boolean gotNameInUseException = false;
        AutoAttendant aa = m_autoAttendantManager.newAutoAttendantWithDefaultGroup();
        aa.setName("alpha");
        try {
            m_autoAttendantManager.storeAutoAttendant(aa);
        } catch (NameInUseException e) {
            gotNameInUseException = true;
        }
        assertTrue(gotNameInUseException);
    }

    public void testGetAutoAttendantSettings() throws Exception {
        TestHelper.cleanInsert("dialplan/seedDialPlanWithAttendant.xml");
        AutoAttendant autoAttendant = m_autoAttendantManager.getAutoAttendant(new Integer(2000));
        assertNotNull(autoAttendant.getSettings());
    }

    public void testSelectSpecial() throws Exception {
        loadDataSet("dialplan/seedOperator.xml");
        AutoAttendant operator = m_autoAttendantManager.getOperator();
        boolean specialMode = m_autoAttendantManager.getSpecialMode();
        m_autoAttendantManager.setAttendantSpecialMode(specialMode, operator);
        commit();
        assertEquals(1, countRowsInTable("attendant_special_mode"));
        Map<String, Object> actual = db().queryForMap("select * from attendant_special_mode");
        assertEquals(1000, actual.get("auto_attendant_id"));
    }

    public void testDeselectSpecial() throws Exception {
        loadDataSet("dialplan/seedOperator.xml");
        AutoAttendant operator = m_autoAttendantManager.getOperator();
        m_autoAttendantManager.deselectSpecial(operator);
        commit();
        assertEquals(1, countRowsInTable("attendant_special_mode"));
        Map<String, Object> actual = db().queryForMap("select * from attendant_special_mode");
        assertEquals(null, actual.get("auto_attendant_id"));
    }

    public void testGetSetSpecialModeFalse() throws Exception {
        m_autoAttendantManager.setAttendantSpecialMode(false, null);
        commit();        
        assertEquals(1, countRowsInTable("attendant_special_mode"));
        Map<String, Object> actualBefore = db().queryForMap("select * from attendant_special_mode");
        assertEquals(false, actualBefore.get("enabled"));
    }
    
    public void testGetSetSpecialModeTrue() throws Exception {
        m_autoAttendantManager.setAttendantSpecialMode(true, null);
        commit();        
        assertEquals(1, countRowsInTable("attendant_special_mode"));
        Map<String, Object> actualAfter = db().queryForMap("select * from attendant_special_mode");
        assertEquals(true, actualAfter.get("enabled"));
    }

    public void testGetSelectedSpecialAttendant() throws Exception {
        loadDataSet("dialplan/seedSpecialSelectedAttendant.xml");
        AutoAttendant operator = m_autoAttendantManager.getOperator();
        AutoAttendant specialAa = m_autoAttendantManager.getSelectedSpecialAttendant();
        assertEquals(operator, specialAa);
    }

    public void setAutoAttendantManager(AutoAttendantManager autoAttendantManager) {
        m_autoAttendantManager = autoAttendantManager;
    }
}
