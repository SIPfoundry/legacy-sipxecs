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


import java.util.Map;

import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class GroupTestIntegration extends IntegrationTestCase {
    private SettingDao m_settingDao;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
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

        m_settingDao.saveGroup(ms);
        commit();
        
        Map<String, Object> actual = db().queryForMap("select * from group_storage as g, setting_value s where g.group_id = s.value_storage_id");
        assertEquals("unittest", actual.get("resource"));
        assertEquals("food", actual.get("name"));
        assertTrue(((Integer) actual.get("weight")) > 0);
        assertEquals("granny smith", actual.get("value"));
        assertEquals("fruit/apple", actual.get("path"));
    }

    public void testUpdate() throws Throwable {
        sql("setting/UpdateGroupSeed.sql");

        SettingSet root = new SettingSet();
        root.addSetting(new SettingSet("fruit")).addSetting(new SettingImpl("apple")).setValue(
                "granny smith");
        root.addSetting(new SettingSet("vegetable")).addSetting(new SettingImpl("pea")).setValue(
                "snow pea");
        root.addSetting(new SettingSet("dairy")).addSetting(new SettingImpl("milk"));

        Group ms = m_settingDao.loadGroup(new Integer(1));

        TestBeanWithSettings bean = new TestBeanWithSettings(root);
        Setting settings = ms.inherhitSettingsForEditing(bean);
        // should make it disappear
        settings.getSetting("fruit/apple").setValue("granny smith");

        // should make it update
        settings.getSetting("vegetable/pea").setValue("snap pea");

        assertEquals(1, ms.getSize());
        m_settingDao.saveGroup(ms);
        commit();

        Map<String, Object> actual = db().queryForMap("select * from group_storage as g, setting_value s where g.group_id = s.value_storage_id");
        assertEquals("unittest", actual.get("resource"));
        assertEquals("food", actual.get("name"));
        assertEquals(1000, actual.get("weight"));
        assertEquals("snap pea", actual.get("value"));
        assertEquals("vegetable/pea", actual.get("path"));
    }

    public void testDuplicateName() throws Exception {
        sql("setting/UpdateGroupSeed.sql");

        Group ms = m_settingDao.loadGroup(new Integer(1));
        Group duplicate = new Group();
        duplicate.setName(ms.getName());
        duplicate.setResource(ms.getResource());

        try {
            m_settingDao.saveGroup(duplicate);
            fail();
        } catch (UserException u) {
            assertTrue(true);
        }
    }

    public void testWeightSequence() throws Exception {
        Group newGroup = new Group();
        newGroup.setResource("unittest");
        newGroup.setName("unittest");
        m_settingDao.saveGroup(newGroup);
        assertNotNull(newGroup.getWeight());
    }

    public void testGetByName() throws Exception {
        sql("setting/UpdateGroupSeed.sql");

        Group byName = m_settingDao.getGroupByName("unittest", "food");
        assertNotNull(byName);
        assertEquals("food", byName.getName());
        assertEquals("unittest", byName.getResource());
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }
}
