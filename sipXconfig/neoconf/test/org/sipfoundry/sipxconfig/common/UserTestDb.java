/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.util.Collection;
import java.util.Collections;
import java.util.Set;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.context.ApplicationContext;
import org.springframework.dao.DataIntegrityViolationException;

public class UserTestDb extends SipxDatabaseTestCase {

    private CoreContext m_core;

    private SettingDao m_settingDao;

    private Integer userId = new Integer(1000);

    protected void setUp() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        m_core = (CoreContext) app.getBean(CoreContext.CONTEXT_BEAN_NAME);
        m_settingDao = (SettingDao) app.getBean(SettingDao.CONTEXT_NAME);

        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testLoadUser() throws Exception {
        TestHelper.insertFlat("common/TestUserSeed.db.xml");

        User user = m_core.loadUser(userId);
        assertEquals(userId, user.getPrimaryKey());
        assertEquals(userId, user.getId());
    }

    public void testSave() throws Exception {
        User user = new User();
        user.setUserName("userid");
        user.setFirstName("FirstName");
        user.setLastName("LastName");
        user.setPintoken("password");
        user.setSipPassword("sippassword");
        user.getAliases().add("1234");
        m_core.saveUser(user);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("common/SaveUserExpected.db.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[user_id]", user.getId());
        expectedRds.addReplacementObject("[null]", null);
        expectedRds.addReplacementObject("[address_book_entry_id]", user.getAddressBookEntry().getId());

        ITable expected = expectedRds.getTable("users");
        ITable actual = TestHelper.getConnection().createQueryTable("users",
                "select * from users where user_name='userid'");

        Assertion.assertEquals(expected, actual);
    }

    public void testUpdateUserName() throws Exception {
        TestHelper.cleanInsertFlat("common/TestUserSeed.db.xml");
        Integer id = new Integer(1000);
        User user = m_core.loadUser(id);
        user.setUserName("foo");
        try {
            m_core.saveUser(user);
            fail("Didn't fail for pintoken change");
        } catch (UserException expected) {
            assertTrue(true);
        }

        // heed assertion and change pintoken
        user.setPin("1234", "fake realm");
        m_core.saveUser(user);
    }

    public void testUpdateAliases() throws Exception {
        TestHelper.cleanInsertFlat("common/TestUserSeed.db.xml");
        assertEquals(1, getConnection().getRowCount("user_alias", "where user_id = 1000"));

        Integer id = new Integer(1000);
        User user = m_core.loadUser(id);
        user.setAliasesString("bongo, kuku");
        m_core.saveUser(user);

        assertEquals(2, getConnection().getRowCount("user_alias", "where user_id = 1000"));
    }

    public void testUserGroups() throws Exception {
        TestHelper.insertFlat("common/UserGroupSeed.db.xml");
        User user = m_core.loadUser(new Integer(1001));
        Set groups = user.getGroups();
        assertEquals(1, groups.size());
    }

    public void testUserSettings() throws Exception {
        TestHelper.insertFlat("common/UserGroupSeed.db.xml");
        User user = m_core.loadUser(new Integer(1001));
        Setting settings = user.getSettings();
        assertNotNull(settings);
    }

    public void testGroupMembers() throws Exception {
        TestHelper.insertFlat("common/UserGroupSeed.db.xml");
        Group group = m_settingDao.getGroup(new Integer(1001));
        Collection users = m_core.getGroupMembers(group);
        assertEquals(1, users.size());
        User actualUser = m_core.loadUser(new Integer(1001));
        User expectedUser = (User) users.iterator().next();
        assertEquals(actualUser.getDisplayName(), expectedUser.getDisplayName());
    }

    public void testGroupMembersNames() throws Exception {
        TestHelper.insertFlat("common/UserGroupSeed.db.xml");
        Group group = m_settingDao.getGroup(new Integer(1001));
        Collection<String> users = m_core.getGroupMembersNames(group);
        assertEquals(1, users.size());
        User actualUser = m_core.loadUser(new Integer(1001));
        String expected = users.iterator().next();
        assertEquals(actualUser.getUserName(), expected);
    }

    public void testDeleteUserGroups() throws Exception {
        TestHelper.insertFlat("common/UserGroupSeed.db.xml");
        m_settingDao.deleteGroups(Collections.singletonList(new Integer(1001)));
        // link table references removed
        ITable actual = TestHelper.getConnection().createDataSet().getTable("user_group");
        assertEquals(0, actual.getRowCount());
    }

    public void testSupervisorSave() throws Exception {
        TestHelper.insertFlat("common/TestUserSeed.db.xml");
        TestHelper.insertFlat("common/UserGroupSeed.db.xml");
        Group group = m_settingDao.getGroup(1001);
        User user = m_core.loadUser(1000);
        assertTrue(user.getSupervisorForGroups().isEmpty());

        user.addSupervisorForGroup(group);

        m_core.saveUser(user);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("common/SaveSupervisorExpected.xml");
        ITable expected = expectedDs.getTable("supervisor");
        ITable actual = TestHelper.getConnection().createQueryTable("supervisor",
                "select * from supervisor");
        Assertion.assertEquals(expected, actual);
    }

    // see: http://track.sipfoundry.org/browse/XCF-1099
    public void testSuperviseMyself() throws Exception {
        TestHelper.insertFlat("common/TestUserSeed.db.xml");
        TestHelper.insertFlat("common/UserGroupSeed.db.xml");
        Group group = m_settingDao.getGroup(1001);
        User user = m_core.loadUser(1001);
        assertTrue(user.getSupervisorForGroups().isEmpty());

        user.addSupervisorForGroup(group);

        m_core.saveUser(user);

        ITable actual = TestHelper.getConnection().createQueryTable("supervisor",
                "select * from supervisor where user_id = 1001");
        assertEquals(1, actual.getRowCount());
    }


    public void testSupervisorSaveNewGroup() throws Exception {
        TestHelper.insertFlat("common/TestUserSeed.db.xml");
        User user = m_core.loadUser(1000);
        Group group = new Group();
        group.setResource(User.GROUP_RESOURCE_ID);
        group.setName("new-supervised-group");

        try {
            user.addSupervisorForGroup(group);
            fail();
        }
        catch(RuntimeException e) {
            // ok
        }
    }

    public void testUserSaveNewGroup() throws Exception {
        TestHelper.insertFlat("common/TestUserSeed.db.xml");
        User user = m_core.loadUser(1000);
        Group group = new Group();
        group.setResource(User.GROUP_RESOURCE_ID);
        group.setName("new-supervised-group");

        user.addGroup(group);
        m_core.saveUser(user);
    }

    public void testDeleteGroupUpdateSupervisor() throws Exception {
        try {
            TestHelper.insertFlat("common/GroupSupervisorSeed.db.xml");
            m_settingDao.deleteGroups(Collections.singletonList(1001));
            assertEquals(0, TestHelper.getConnection().getRowCount("supervisor"));
            assertEquals(0, TestHelper.getConnection().getRowCount("group_storage"));
        } catch (DataIntegrityViolationException e) {
            fail();
        }
    }

    public void testIsSupervisor() throws Exception {
        TestHelper.insertFlat("common/GroupSupervisorSeed.db.xml");
        User supervisor = m_core.loadUser(1001);
        assertTrue(supervisor.isSupervisor());

        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.insertFlat("common/TestUserSeed.db.xml");
        User peon = m_core.loadUser(1000);
        assertFalse(peon.isSupervisor());
    }

    public void testUpdateSuperviser() throws Exception {
        TestHelper.insertFlat("common/GroupSupervisorSeed.db.xml");
        User supervisor = m_core.loadUser(1001);

        Group newGroup = m_core.getGroupByName("group2", true);
        supervisor.clearSupervisorForGroups();
        supervisor.addSupervisorForGroup(newGroup);
        m_core.saveUser(supervisor);

        ITable actual = TestHelper.getConnection().createQueryTable("x", "select * from supervisor");
        assertEquals(1, actual.getRowCount());
        assertEquals(supervisor.getId(), actual.getValue(0, "user_id"));
        assertEquals(newGroup.getId(), actual.getValue(0, "group_id"));
    }
}
