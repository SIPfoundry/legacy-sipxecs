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
import java.util.Map;
import java.util.Set;

import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.sipxconfig.commserver.imdb.ImdbTestCase;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.dao.DataIntegrityViolationException;

public class UserTestIntegration extends ImdbTestCase {
    private CoreContext m_coreContext;
    private SettingDao m_settingDao;
    private Integer userId = new Integer(1000);
    private ValidUsers m_validUsers;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        sql("commserver/SeedLocations.sql");
    }

    public void testLoadUser() throws Exception {
        sql("common/TestUserSeed.sql");

        User user = m_coreContext.loadUser(userId);
        assertEquals(userId, user.getPrimaryKey());
        assertEquals(userId, user.getId());
    }

    public void testSave() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        User user = m_coreContext.newUser();
        user.setUserName("userid");
        user.setFirstName("FirstName");
        user.setLastName("LastName");
        user.setPintoken("password");
        user.setSipPassword("sippassword");
        user.getAliases().add("1234");
        m_coreContext.saveUser(user);
        commit();
        assertEquals(1, countRowsInTable("users"));
        assertEquals(1, countRowsInTable("user_alias"));
        Map<String, Object> actual = db().queryForMap("select * from users as u, user_alias as a where u.user_id = a.user_id");
        assertEquals("FirstName", actual.get("first_name"));
        assertEquals("userid", actual.get("user_name"));
        assertEquals(false, actual.get("is_shared"));
        assertEquals("password", actual.get("pintoken"));
        assertEquals("sippassword", actual.get("sip_password"));
        assertEquals("C", actual.get("user_type"));
        assertEquals("1234", actual.get("alias"));
    }

    public void testUpdateUserName() throws Exception {
        sql("common/TestUserSeed.sql");
        commit();
        User user = m_coreContext.loadUser(1000);
        user.setUserName("foo");
        try {
            // commit because core context loads a different hibernate template to load old value from db
            m_coreContext.saveUser(user);
            fail("Didn't fail for pintoken change");
        } catch (UserException expected) {
            assertTrue(true);
        }

        // heed assertion and change pintoken
        user.setPin("12345");
        m_coreContext.saveUser(user);
    }

    public void testUpdateAliases() throws Exception {
        getProfilesDb().dropCollection("userProfile");
        sql("common/TestUserSeed.sql");
        assertEquals(1, getConnection().getRowCount("user_alias", "where user_id = 1000"));

        Integer id = new Integer(1000);
        User user = m_coreContext.loadUser(id);
        user.setAliasesString("bongo, kuku");
        m_coreContext.saveUser(user);

        assertEquals(2, getConnection().getRowCount("user_alias", "where user_id = 1000"));
    }

    public void testUserGroups() throws Exception {
        sql("common/UserGroupSeed.sql");
        User user = m_coreContext.loadUser(new Integer(1001));
        Set groups = user.getGroups();
        assertEquals(1, groups.size());
    }

    public void testUserSettings() throws Exception {
        sql("common/UserGroupSeed.sql");
        User user = m_coreContext.loadUser(new Integer(1001));
        Setting settings = user.getSettings();
        assertNotNull(settings);
    }

    public void testGroupMembers() throws Exception {
        sql("common/UserGroupSeed.sql");
        Group group = m_settingDao.getGroup(new Integer(1001));
        Collection users = m_coreContext.getGroupMembers(group);
        assertEquals(1, users.size());
        User actualUser = m_coreContext.loadUser(new Integer(1001));
        User expectedUser = (User) users.iterator().next();
        assertEquals(actualUser.getDisplayName(), expectedUser.getDisplayName());
    }

    public void testGroupMembersNames() throws Exception {
        sql("common/UserGroupSeed.sql");
        Group group = m_settingDao.getGroup(new Integer(1001));
        Collection<String> users = m_coreContext.getGroupMembersNames(group);
        assertEquals(1, users.size());
        User actualUser = m_coreContext.loadUser(new Integer(1001));
        String expected = users.iterator().next();
        assertEquals(actualUser.getUserName(), expected);
    }

    public void testDeleteUserGroups() throws Exception {
        sql("common/UserGroupSeed.sql");
        m_settingDao.deleteGroups(Collections.singletonList(new Integer(1001)));
        // link table references removed
        assertEquals(0, countRowsInTable("user_group"));
    }

    public void testSupervisorSave() throws Exception {
        sql("common/TestUserSeed.sql");
        sql("common/UserGroupSeed.sql");
        Group group = m_settingDao.getGroup(1001);
        User user = m_coreContext.loadUser(1000);
        assertTrue(user.getSupervisorForGroups().isEmpty());

        user.addSupervisorForGroup(group);

        m_coreContext.saveUser(user);
        commit();
        assertEquals(1, countRowsInTable("supervisor"));
        Map<String, Object> actual = db().queryForMap("select * from supervisor");
        assertEquals(1000, actual.get("user_id"));
        assertEquals(1001, actual.get("group_id"));
    }

    // see: http://track.sipfoundry.org/browse/XCF-1099
    public void testSuperviseMyself() throws Exception {
        sql("common/TestUserSeed.sql");
        sql("common/UserGroupSeed.sql");
        Group group = m_settingDao.getGroup(1001);
        User user = m_coreContext.loadUser(1001);
        assertTrue(user.getSupervisorForGroups().isEmpty());

        user.addSupervisorForGroup(group);

        m_coreContext.saveUser(user);

        assertEquals(1, db().queryForLong("select count(*) from supervisor where user_id = 1001"));
    }

    public void testSupervisorSaveNewGroup() throws Exception {
        sql("common/TestUserSeed.sql");
        User user = m_coreContext.loadUser(1000);
        Group group = new Group();
        group.setResource(User.GROUP_RESOURCE_ID);
        group.setName("new-supervised-group");

        try {
            user.addSupervisorForGroup(group);
            fail();
        } catch (RuntimeException e) {
            // ok
        }
    }

    public void testUserSaveNewGroup() throws Exception {
        sql("common/TestUserSeed.sql");
        User user = m_coreContext.loadUser(1000);
        Group group = new Group();
        group.setResource(User.GROUP_RESOURCE_ID);
        group.setName("new-supervised-group");

        user.addGroup(group);
        m_coreContext.saveUser(user);
    }

    public void testDeleteGroupUpdateSupervisor() throws Exception {
        try {
            sql("common/GroupSupervisorSeed.sql");
            m_settingDao.deleteGroups(Collections.singletonList(1001));
            assertEquals(0, countRowsInTable("supervisor"));
            assertEquals(0, countRowsInTable("group_storage"));
        } catch (DataIntegrityViolationException e) {
            fail();
        }
    }

    public void testIsSupervisor() throws Exception {
        sql("common/GroupSupervisorSeed.sql");
        User supervisor = m_coreContext.loadUser(1001);
        assertTrue(supervisor.isSupervisor());
    }

    public void testIsNotSupervisor() throws Exception {
        sql("common/TestUserSeed.sql");
        User peon = m_coreContext.loadUser(1000);
        assertFalse(peon.isSupervisor());
    }

    public void testUpdateSuperviser() throws Exception {
        sql("common/GroupSupervisorSeed.sql");
        User supervisor = m_coreContext.loadUser(1001);

        Group newGroup = m_coreContext.getGroupByName("group2", true);
        supervisor.clearSupervisorForGroups();
        supervisor.addSupervisorForGroup(newGroup);
        m_coreContext.saveUser(supervisor);

        commit();
        assertEquals(1, countRowsInTable("supervisor"));
        Map<String, Object> actual = db().queryForMap("select * from supervisor");
        assertEquals(supervisor.getId(), actual.get("user_id"));
        assertEquals(newGroup.getId(), actual.get("group_id"));
    }

    public void testAvailableGroup() {
        loadDataSet("setting/user-group-branch.xml");
        User user6 = m_coreContext.loadUser(1005);
        Group group1 = m_settingDao.loadGroup(1000);
        Group group2 = m_settingDao.loadGroup(1001);
        Group group3 = m_settingDao.loadGroup(1002);
        Group group4 = m_settingDao.loadGroup(1003);
        assertFalse(user6.isGroupAvailable(group1));
        assertTrue(user6.isGroupAvailable(group3));
        assertTrue(user6.isGroupAvailable(group4));
        User user2 = m_coreContext.loadUser(1001);
        user2.isGroupAvailable(group2);
    }

    public void testLastUpdated() throws Exception {
        loadDataSetXml("commserver/seedLocationsAndServices.xml");
        Long past = System.currentTimeMillis();
        User u1 = m_coreContext.newUser();
        u1.setUserName("u1");
        m_coreContext.saveUser(u1);

        assertTrue(!m_validUsers.getUsersUpdatedAfter(past).isEmpty());

        Long now = System.currentTimeMillis();
        assertTrue(m_validUsers.getUsersUpdatedAfter(now).isEmpty());

        m_coreContext.saveUser(u1);
        assertTrue(!m_validUsers.getUsersUpdatedAfter(now).isEmpty());

    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    public void setValidUsers(ValidUsers validUsers) {
        m_validUsers = validUsers;
    }
}
