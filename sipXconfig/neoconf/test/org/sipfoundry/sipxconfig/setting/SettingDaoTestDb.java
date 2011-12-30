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

import java.util.List;

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class SettingDaoTestDb extends IntegrationTestCase {
    private SettingDao m_settingDao;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    public void testSettingGroup() throws Exception {
        ValueStorage vs = new ValueStorage();
        vs.setSettingValue(new SettingImpl("setting"), new SettingValueImpl("some value"), null);
        m_settingDao.storeValueStorage(vs);
    }

    public void testGetGroupsByString() throws Exception {
        List<Group> groups = m_settingDao.getGroupsByString("foo", " g0 g1   g2 \t g3 \n g4 ", false);        
        assertEquals(5, groups.size());
        assertEquals("g0", groups.get(0).getName());
        assertEquals("g4", groups.get(4).getName());
        commit();
        assertEquals(0, countRowsInTable("group_storage"));
    }

    public void testGetGroupsByStringAndSave() throws Exception {
        List<Group> groups = m_settingDao.getGroupsByString("foo", " g0 g1   g2 \t g3 \n g4 ", true);
        assertEquals(5, groups.size());
        commit();
        assertEquals(5, countRowsInTable("group_storage"));
    }

    public void testGetGroupsByEmptyString() throws Exception {
        assertEquals(0, m_settingDao.getGroupsByString("foo", "", false).size());
        assertEquals(0, m_settingDao.getGroupsByString("foo", "  ", false).size());
        assertEquals(0, m_settingDao.getGroupsByString("foo", null, false).size());
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }
}
