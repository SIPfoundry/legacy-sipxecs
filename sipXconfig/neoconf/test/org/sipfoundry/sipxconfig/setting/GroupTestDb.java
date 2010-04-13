/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.context.ApplicationContext;

public class GroupTestDb extends SipxDatabaseTestCase {

    private SettingDao m_dao;

    protected void setUp() throws Exception {
        ApplicationContext context = TestHelper.getApplicationContext();
        m_dao = (SettingDao) context.getBean("settingDao");
    }

    static class TestBeanWithSettings extends BeanWithSettings {
        private SettingSet m_set;
        TestBeanWithSettings(SettingSet set) {
            m_set = set;
        }
        protected Setting loadSettings() {
            return m_set;
        }
    }

    public void testSave() throws Throwable {
        TestHelper.cleanInsert("ClearDb.xml");

        SettingSet root = new SettingSet();
        root.addSetting(new SettingSet("fruit")).addSetting(new SettingImpl("apple"));
        root.addSetting(new SettingSet("vegetable")).addSetting(new SettingImpl("pea"));

        Group ms = new Group();
        ms.setResource("unittest");
        ms.setName("food");

        TestBeanWithSettings bean = new TestBeanWithSettings(root);
        Setting settings = ms.inherhitSettingsForEditing(bean);

        settings.getSetting("fruit/apple").setValue("granny smith");
        settings.getSetting("vegetable/pea").setValue(null);

        m_dao.saveGroup(ms);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("setting/SaveGroupExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[group_id]", ms.getId());

        ITable expected = expectedRds.getTable("setting_value");

        ITable actual = TestHelper.getConnection().createDataSet().getTable("setting_value");

        Assertion.assertEquals(expected, actual);
    }

    public void testUpdate() throws Throwable {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("setting/UpdateGroupSeed.xml");

        SettingSet root = new SettingSet();
        root.addSetting(new SettingSet("fruit")).addSetting(new SettingImpl("apple")).setValue(
                "granny smith");
        root.addSetting(new SettingSet("vegetable")).addSetting(new SettingImpl("pea")).setValue(
                "snow pea");
        root.addSetting(new SettingSet("dairy")).addSetting(new SettingImpl("milk"));

        Group ms = m_dao.loadGroup(new Integer(1));

        TestBeanWithSettings bean = new TestBeanWithSettings(root);
        Setting settings = ms.inherhitSettingsForEditing(bean);
        // should make it disappear
        settings.getSetting("fruit/apple").setValue("granny smith");

        // should make it update
        settings.getSetting("vegetable/pea").setValue("snap pea");

        assertEquals(1, ms.getSize());
        m_dao.saveGroup(ms);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("setting/UpdateGroupExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[null]", null);
        ITable expected = expectedRds.getTable("setting_value");

        ITable actual = TestHelper.getConnection().createDataSet().getTable("setting_value");

        Assertion.assertEquals(expected, actual);
    }

    public void testDuplicateName() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("setting/UpdateGroupSeed.xml");

        Group ms = m_dao.loadGroup(new Integer(1));
        Group duplicate = new Group();
        duplicate.setName(ms.getName());
        duplicate.setResource(ms.getResource());

        try {
            m_dao.saveGroup(duplicate);
            fail();
        } catch (UserException u) {
            assertTrue(true);
        }
    }

    public void testWeightSequence() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        Group newGroup = new Group();
        newGroup.setResource("unittest");
        newGroup.setName("unittest");
        m_dao.saveGroup(newGroup);
        assertNotNull(newGroup.getWeight());
    }

    public void testGetByName() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("setting/UpdateGroupSeed.xml");

        Group byName = m_dao.getGroupByName("unittest", "food");
        assertNotNull(byName);
        assertEquals("food", byName.getName());
        assertEquals("unittest", byName.getResource());
    }
}
