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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.phonebook.Address;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.context.ApplicationContext;

public class CoreContextImplTestDb extends SipxDatabaseTestCase {

    private static final int NUM_USERS = 10;

    private CoreContext m_core;
    private SettingDao m_settingDao;

    @Override
    protected void setUp() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        m_core = (CoreContext) app.getBean(CoreContext.CONTEXT_BEAN_NAME);
        m_settingDao = (SettingDao) app.getBean(SettingDao.CONTEXT_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testLoadByUserName() throws Exception {
        TestHelper.insertFlat("common/UserSearchSeed.xml");

        assertNotNull(m_core.loadUserByUserName("userseed5"));
        assertNull(m_core.loadUserByUserName("wont find this guy"));
    }

    public void testLoadByAlias() throws Exception {
        TestHelper.insertFlat("common/UserSearchSeed.xml");

        User user = m_core.loadUserByAlias("2");
        assertNotNull(user);
        assertEquals("userseed2", user.getUserName());
        user = m_core.loadUserByAlias("two");
        assertNotNull(user);
        assertEquals("userseed2", user.getUserName());
        user = m_core.loadUserByAlias("5");
        assertNotNull(user);
        assertEquals("userseed5", user.getUserName());
        assertNull(m_core.loadUserByAlias("666"));
        assertNull(m_core.loadUserByAlias("userseed2"));
    }

    public void testLoadUserByUserNameOrAlias() throws Exception {
        TestHelper.insertFlat("common/UserSearchSeed.xml");

        User user = m_core.loadUserByUserNameOrAlias("2");
        assertNotNull(user);
        assertEquals("userseed2", user.getUserName());

        user = m_core.loadUserByUserNameOrAlias("two");
        assertNotNull(user);
        assertEquals("userseed2", user.getUserName());

        user = m_core.loadUserByUserNameOrAlias("5");
        assertNotNull(user);
        assertEquals("userseed5", user.getUserName());

        assertNull(m_core.loadUserByUserNameOrAlias("666"));

        user = m_core.loadUserByUserNameOrAlias("userseed2");
        assertNotNull(user);
        assertEquals("userseed2", user.getUserName());

        // There was a bug earlier where only users who had aliases could be loaded.
        // Test that a user without aliases can be loaded.
        user = m_core.loadUserByUserNameOrAlias("userwithnoaliases");
        assertNotNull(user);
        assertEquals("userwithnoaliases", user.getUserName());
    }

    public void testCheckForDuplicateNameOrAlias() throws Exception {
        TestHelper.insertFlat("common/UserSearchSeed.xml");
        final String UNIQUE_NAME = "uniqueNameThatDoesntExistInTheUniverseOrBeyond";

        // Check that a user with a unique name won't collide with any existing users
        User user = new User();
        user.setUserName(UNIQUE_NAME);
        assertNull(m_core.checkForDuplicateNameOrAlias(user));

        // Check that a user with a duplicate name is found to be a duplicate
        user.setUserName("userseed4");
        assertEquals("userseed4", m_core.checkForDuplicateNameOrAlias(user));

        // Check that a user with an alias that matches an existing username
        // is found to be a duplicate
        user.setUserName(UNIQUE_NAME);
        user.setAliasesString("userseed6");
        assertEquals("userseed6", m_core.checkForDuplicateNameOrAlias(user));

        // Check that a user with an alias that matches an existing alias
        // is found to be a duplicate
        user.setAliasesString("two");
        assertEquals("two", m_core.checkForDuplicateNameOrAlias(user));

        // Check that duplicate checking doesn't get confused by multiple collisions
        // with both usernames and aliases
        user.setUserName("userseed4");
        user.setAliasesString("two,1");
        assertNotNull(m_core.checkForDuplicateNameOrAlias(user));

        // Check that collisions internal to the user are caught.
        // Note that duplicates within the aliases list are ignored.
        final String WASAWUSU = "wasawusu";
        user.setUserName(WASAWUSU);
        user.setAliasesString(WASAWUSU);
        assertEquals(WASAWUSU, m_core.checkForDuplicateNameOrAlias(user));
        user.setUserName(UNIQUE_NAME);
        user.setAliasesString(WASAWUSU + "," + WASAWUSU);
        assertNull(m_core.checkForDuplicateNameOrAlias(user));
    }

    public void testSearchByUserName() throws Exception {
        TestHelper.insertFlat("common/UserSearchSeed.xml");

        User template = new User();
        template.setUserName("userseed");
        List users = m_core.loadUserByTemplateUser(template);

        assertEquals(7, users.size());
    }

    public void testSearchByAlias() throws Exception {
        TestHelper.insertFlat("common/UserSearchSeed.xml");

        User template = new User();
        template.setUserName("two");
        List users = m_core.loadUserByTemplateUser(template);
        assertEquals(1, users.size());
        User user = (User) users.get(0);
        assertEquals("userseed2", user.getUserName());
    }

    public void testSearchByFirstName() throws Exception {
        TestHelper.insertFlat("common/UserSearchSeed.xml");

        User template = new User();
        template.setFirstName("user4");
        List users = m_core.loadUserByTemplateUser(template);
        assertEquals(1, users.size());
        User user = (User) users.get(0);
        assertEquals("userseed4", user.getUserName());
    }

    public void testSearchByLastName() throws Exception {
        TestHelper.insertFlat("common/UserSearchSeed.xml");

        User template = new User();
        template.setLastName("seed5");
        List users = m_core.loadUserByTemplateUser(template);
        assertEquals(1, users.size());
        User user = (User) users.get(0);
        assertEquals("userseed5", user.getUserName());
    }

    // Test that we are doing a logical "AND" not "OR". There is a user with
    // the specified first name, and a different user with the specified last
    // name. Because of the "AND" we should get no matches.
    public void testSearchByFirstAndLastName() throws Exception {
        TestHelper.insertFlat("common/UserSearchSeed.xml");

        User template = new User();
        template.setFirstName("user4");
        template.setLastName("seed5");
        List users = m_core.loadUserByTemplateUser(template);
        assertEquals(0, users.size());
    }

    public void testSearchFormBlank() throws Exception {
        TestHelper.insertFlat("common/UserSearchSeed.xml");

        User template = new User();
        template.setFirstName("");
        List users = m_core.loadUserByTemplateUser(template);

        assertEquals(NUM_USERS, users.size());
    }

    @SuppressWarnings("deprecation")
    public void testLoadUsers() throws Exception {
        TestHelper.insertFlat("common/UserSearchSeed.xml");

        List users = m_core.loadUsers();

        // Check that we have the expected number of users
        assertEquals(NUM_USERS, users.size());
    }

    public void testDeleteUsers() throws Exception {
        TestHelper.insertFlat("common/UserSearchSeed.xml");

        // Check that we have the expected number of users
        ITable usersTable = TestHelper.getConnection().createDataSet().getTable("users");
        assertEquals(11, usersTable.getRowCount());

        // Delete two users
        List usersToDelete = new ArrayList();
        usersToDelete.add(1001);
        usersToDelete.add(1002);
        m_core.deleteUsers(usersToDelete);

        // We should have reduced the user count by two
        usersTable = TestHelper.getConnection().createDataSet().getTable("users");
        assertEquals(9, usersTable.getRowCount());
    }

    public void testDeleteUsersByUserName() throws Exception {
        TestHelper.insertFlat("common/UserSearchSeed.xml");

        // Check that we have the expected number of users
        ITable usersTable = TestHelper.getConnection().createDataSet().getTable("users");
        assertEquals(11, usersTable.getRowCount());

        // Delete two users
        List<String> usersToDelete = new ArrayList();
        usersToDelete.add("userseed1");
        usersToDelete.add("userseed2");
        m_core.deleteUsersByUserName(usersToDelete);

        // We should have reduced the user count by two
        usersTable = TestHelper.getConnection().createDataSet().getTable("users");
        assertEquals(9, usersTable.getRowCount());
    }

    public void testLoadGroups() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.insertFlat("common/UserGroupSeed.db.xml");
        List groups = m_core.getGroups();
        assertEquals(1, groups.size());
        Group group = (Group) groups.get(0);
        assertEquals("SeedUserGroup1", group.getName());
    }

    public void testGetGroupByName() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.insertFlat("common/UserGroupSeed.db.xml");

        Group g1 = m_core.getGroupByName("SeedUserGroup1", false);
        assertNotNull(g1);
        assertEquals("SeedUserGroup1", g1.getName());

        Group g2 = m_core.getGroupByName("bongo", false);
        assertNull(g2);
        assertEquals(1, getConnection().getRowCount("group_storage"));

        g2 = m_core.getGroupByName("bongo", true);
        assertNotNull(g2);
        assertEquals("bongo", g2.getName());

        assertEquals(2, getConnection().getRowCount("group_storage"));
    }

    public void testAliases() throws Exception {
        Collection userAliases = m_core.getAliasMappings();
        assertEquals(0, userAliases.size());

        TestHelper.insertFlat("common/TestUserSeed.db.xml");

        userAliases = m_core.getAliasMappings();
        assertEquals(1, userAliases.size());
    }

    public void testClear() throws Exception {
        TestHelper.insertFlat("common/TestUserSeed.db.xml");
        m_core.clear();
        ITable t = TestHelper.getConnection().createDataSet().getTable("users");
        assertEquals(0, t.getRowCount());
    }

    public void testCreateAdminGroupAndInitialUserTask() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        m_core.createAdminGroupAndInitialUserTask();

        User admin = m_core.loadUserByUserName(User.SUPERADMIN);
        Group adminGroup = admin.getGroups().iterator().next();
        IDataSet expectedDs = TestHelper.loadDataSetFlat("common/CreateAdminAndInitialUserExpected.db.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[user_id]", admin.getId());
        expectedRds.addReplacementObject("[group_id]", adminGroup.getId());
        expectedRds.addReplacementObject("[weight]", adminGroup.getWeight());
        expectedRds.addReplacementObject("[sip_password]", admin.getSipPassword());
        expectedRds.addReplacementObject("[is_shared]", admin.getIsShared());
        expectedRds.addReplacementObject("[null]", null);

        IDataSet actualDs = TestHelper.getConnection().createDataSet();

        Assertion.assertEquals(expectedRds.getTable("users"), actualDs.getTable("users"));
        Assertion.assertEquals(expectedRds.getTable("group_storage"), actualDs.getTable("group_storage"));
        Assertion.assertEquals(expectedRds.getTable("setting_value"), actualDs.getTable("setting_value"));

        // make sure that it works even if superadmin has its rights revoked
        admin.setPermission(PermissionName.SUPERADMIN, false);
        m_core.saveUser(admin);
        assertFalse(admin.isAdmin());
        m_core.createAdminGroupAndInitialUserTask();

        admin = m_core.loadUserByUserName(User.SUPERADMIN);
        assertTrue(admin.isAdmin());
    }

    public void testCountUsers() throws Exception {
        TestHelper.insertFlat("common/UserCountSeed.xml");
        assertEquals(4, m_core.getUsersCount());
        assertEquals(7, m_core.getAllUsersCount());
        assertEquals(3, m_core.loadInternalUsers().size());
    }

    public void testCountUsersInGroup() throws Exception {
        TestHelper.insertFlat("common/UserSearchSeed.xml");
        assertEquals(2, m_core.getUsersInGroupCount(1001));
        assertEquals(3, m_core.getUsersInGroupCount(1002));
    }

    public void testCountUsersInGroupWithSearch() throws Exception {
        TestHelper.insertFlat("common/SampleUsersSeed.xml");
        assertEquals(1, m_core.getUsersInGroupWithSearchCount(1001, "pha"));
        assertEquals(1, m_core.getUsersInGroupWithSearchCount(1002, "add"));
        assertEquals(2, m_core.getUsersInGroupWithSearchCount(1003, "l"));
    }

    public void testLoadUserPage() throws Exception {
        TestHelper.insertFlat("common/SampleUsersSeed.xml");
        Collection page = m_core.loadUsersByPage(null, null, null, 0, 2, "userName", true);
        assertEquals(2, page.size());
        User u = (User) page.iterator().next();
        assertEquals("alpha", u.getUserName());

        Collection next = m_core.loadUsersByPage(null, null, null, 2, 2, "userName", true);
        assertEquals(2, next.size());
        User nextUser = (User) next.iterator().next();
        assertEquals("charlie", nextUser.getUserName());
    }

    public void testLoadUserPageDescending() throws Exception {
        TestHelper.insertFlat("common/SampleUsersSeed.xml");
        // expect third user from bottom
        Collection page = m_core.loadUsersByPage(null, null, null, 2, 2, "userName", false);
        User u = (User) page.iterator().next();
        assertEquals("horatio", u.getUserName());
    }

    public void testLoadUserPageOrderByFirstName() throws Exception {
        TestHelper.insertFlat("common/SampleUsersSeed.xml");
        // try sorting by last name
        Collection page = m_core.loadUsersByPage(null, null, null, 2, 2, "firstName", true);
        User u = (User) page.iterator().next();
        assertEquals("kyle", u.getUserName());
    }

    public void testLoadUserPageWithGroup() throws Exception {
        TestHelper.insertFlat("common/SampleUsersSeed.xml");
        Collection page = m_core.loadUsersByPage(null, 1001, null, 0, 10, "userName", true);
        assertEquals(1, page.size());
        User u = (User) page.iterator().next();
        assertEquals("alpha", u.getUserName());
    }

    public void testLoadUserPageWithUserSearch() throws Exception {
        TestHelper.insertFlat("common/SampleUsersSeed.xml");
        Collection page = m_core.loadUsersByPage("og", null, null, 0, 10, "userName", true);
        assertEquals(6, page.size());
        User u = (User) page.iterator().next();
        assertEquals("alpha", u.getUserName());

        page = m_core.loadUsersByPage("og", null, null, 2, 2, "userName", true);
        assertEquals(2, page.size());
        u = (User) page.iterator().next();
        assertEquals("elephant", u.getUserName());

        page = m_core.loadUsersByPage("og", null, null, 4, 2, "userName", true);
        assertEquals(2, page.size());
        u = (User) page.iterator().next();
        assertEquals("gogo", u.getUserName());

        page = m_core.loadUsersByPage("og", null, null, 6, 2, "userName", true);
        assertEquals(0, page.size());
    }

    public void testLoadUserPageWithUserSearchAndGroup() throws Exception {
        TestHelper.insertFlat("common/SampleUsersSeed.xml");
        Collection page = m_core.loadUsersByPage("og", 1003, null, 0, 10, "userName", true);
        assertEquals(1, page.size());
        User u = (User) page.iterator().next();
        assertEquals("gogo", u.getUserName());
    }

    public void testLoadUserPageWithUserSearchCaseInsensitive() throws Exception {
        TestHelper.insertFlat("common/SampleUsersSeed.xml");
        Collection page = m_core.loadUsersByPage("mamba", null, null, 0, 10, "userName", true);
        assertEquals(1, page.size());
        User u = (User) page.iterator().next();
        assertEquals("kyle", u.getUserName());
    }

    public void testIsAliasInUse() throws Exception {
        TestHelper.insertFlat("common/SampleUsersSeed.xml");
        assertTrue(m_core.isAliasInUse("janus")); // a user ID
        assertTrue(m_core.isAliasInUse("dweezil")); // a user alias
        assertFalse(m_core.isAliasInUse("jessica")); // a first name
    }

    public void testGetBeanIdsOfObjectsWithAlias() throws Exception {
        TestHelper.insertFlat("common/SampleUsersSeed.xml");
        assertTrue(m_core.getBeanIdsOfObjectsWithAlias("janus").size() == 1); // a user ID
        assertTrue(m_core.getBeanIdsOfObjectsWithAlias("dweezil").size() == 1); // a user alias
        assertTrue(m_core.getBeanIdsOfObjectsWithAlias("jessica").size() == 0); // a first name
    }

    public void testGetGroupSupervisors() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.insertFlat("common/GroupSupervisorSeed.db.xml");
        Group group = m_settingDao.getGroup(1001);
        List<User> supervisors = m_core.getGroupSupervisors(group);
        assertEquals(1, supervisors.size());
        assertEquals((Integer) 1001, supervisors.get(0).getId());
    }

    public void testUsersThatISupervise() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.insertFlat("common/UsersThatISupervise.db.xml");
        User supervisor = m_core.loadUser(2001);

        List<User> peons = m_core.getUsersThatISupervise(supervisor);
        assertEquals(3, peons.size());
        assertEquals("peon1", peons.get(0).getUserName());
        assertEquals("peon2", peons.get(1).getUserName());
        assertEquals("peon5", peons.get(2).getUserName());
    }

    public void testCheckForValidExtensions() throws Exception {
        TestHelper.insertFlat("common/UserSearchSeed.xml");

        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        User user = m_core.loadUserByAlias("2");
        assertNotNull(user);
        user.setPermissionManager(pm);
        user.setPermission(PermissionName.VOICEMAIL, false);
        m_core.saveUser(user);

        user = m_core.loadUserByAlias("3");
        assertNotNull(user);
        user.setPermissionManager(pm);
        user.setPermission(PermissionName.VOICEMAIL, true);
        m_core.saveUser(user);

        // An blank lists should have no invalid exceptions
        Set<String> empty = Collections.emptySet();
        m_core.checkForValidExtensions(empty, PermissionName.VOICEMAIL);

        // User with extension "10" doesn't exist
        try {
            m_core.checkForValidExtensions(Arrays.asList("10"), PermissionName.VOICEMAIL);
            fail();
        } catch (UserException e) {
            assertTrue(true);
        }

        // User with extension "2" exist, but doesn't have voicemail permission
        try {
            m_core.checkForValidExtensions(Arrays.asList("2"), PermissionName.VOICEMAIL);
            fail();
        } catch (UserException e) {
            assertTrue(true);
        }

        // User with extension "3" and voicemail permission;
        m_core.checkForValidExtensions(Arrays.asList("3"), PermissionName.VOICEMAIL);
        try {
            m_core.checkForValidExtensions(Arrays.asList("2", "3", "10"), PermissionName.VOICEMAIL);
            fail();
        } catch (UserException e) {
            assertTrue(e.getMessage().contains("2"));
            assertTrue(e.getMessage().contains("10"));
            assertFalse(e.getMessage().contains("3"));
        }
    }

    public void testGetSpecialUsers() {
        m_core.initializeSpecialUsers();

        User user = m_core.getSpecialUser(SpecialUserType.MEDIA_SERVER);
        assertEquals("~~id~media", user.getName());
        assertTrue(user.getSipPassword().length() >= 10);
        user = m_core.getSpecialUser(SpecialUserType.PARK_SERVER);
        assertEquals("~~id~park", user.getName());
        assertTrue(user.getSipPassword().length() >= 10);
        user = m_core.getSpecialUser(SpecialUserType.REGISTRAR_SERVER);
        assertEquals("~~id~registrar", user.getName());
        assertTrue(user.getSipPassword().length() >= 10);
    }

    public void testGetSharedUsers() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.insertFlat("common/SharedUsersSeed.xml");
        List<User> sharedUsers = m_core.getSharedUsers();
        assertEquals(1, sharedUsers.size());
        assertEquals((Integer) 1001, sharedUsers.get(0).getId());
    }

    public void testSaveAddressBook() throws Exception {
        TestHelper.insertFlat("common/UserSearchSeed.xml");

        User user = m_core.loadUser(1001);
        assertNotNull(user);

        AddressBookEntry addressBook = user.getAddressBookEntry();
        assertNotNull(addressBook);
        addressBook.setJobTitle("Data Entry Assistant");
        addressBook.setJobDept("Data Management Services");
        addressBook.setCompanyName("Museum of Science");
        addressBook.setCellPhoneNumber("(617) 723-2500");
        addressBook.setFaxNumber("617-589-0362");

        Address address = addressBook.getOfficeAddress();
        assertNotNull(address);
        address.setStreet("1 Science Park");
        address.setZip("02114");
        address.setCity("Boston");
        address.setCountry("US");
        address.setState("MA");
        addressBook.setOfficeAddress(address);

        user.setAddressBookEntry(addressBook);
        m_core.saveUser(user);

        user = m_core.loadUser(1001);
        assertNotNull(user);
        assertEquals("Museum of Science", user.getAddressBookEntry().getCompanyName());
        assertEquals("(617) 723-2500", user.getAddressBookEntry().getCellPhoneNumber());
        assertEquals("Data Entry Assistant", user.getAddressBookEntry().getJobTitle());
        assertEquals("Boston", user.getAddressBookEntry().getOfficeAddress().getCity());
        assertEquals("1 Science Park", user.getAddressBookEntry().getOfficeAddress().getStreet());
    }
}
