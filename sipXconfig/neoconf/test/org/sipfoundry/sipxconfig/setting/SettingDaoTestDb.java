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

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class SettingDaoTestDb extends SipxDatabaseTestCase {

    SettingDao dao;

    protected void setUp() {
        ApplicationContext context = TestHelper.getApplicationContext();
        dao = (SettingDao) context.getBean("settingDao");
    }

    public void testSettingGroup() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");

        ValueStorage vs = new ValueStorage();
        vs.setSettingValue(new SettingImpl("setting"), new SettingValueImpl("some value"), null);

        dao.storeValueStorage(vs);
    }

    public void testGetGroupsByString() throws Exception {
        List<Group> groups = dao.getGroupsByString("foo", " g0 g1   g2 \t g3 \n g4 ", false);
        assertEquals(5, groups.size());
        assertEquals("g0", groups.get(0).getName());
        assertEquals("g4", groups.get(4).getName());
        assertEquals(0, TestHelper.getConnection().getRowCount("group_storage", "where resource='foo'"));
    }

    public void testGetGroupsByStringAndSave() throws Exception {
        List<Group> groups = dao.getGroupsByString("foo", " g0 g1   g2 \t g3 \n g4 ", true);
        assertEquals(5, groups.size());
        assertEquals(5, TestHelper.getConnection().getRowCount("group_storage", "where resource='foo'"));
    }

    public void testGetGroupsByEmptyString() throws Exception {
        assertEquals(0, dao.getGroupsByString("foo", "", false).size());
        assertEquals(0, dao.getGroupsByString("foo", "  ", false).size());
        assertEquals(0, dao.getGroupsByString("foo", null, false).size());
    }
}
