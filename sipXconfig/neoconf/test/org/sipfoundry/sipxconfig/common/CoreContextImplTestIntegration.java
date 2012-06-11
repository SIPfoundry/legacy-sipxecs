/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.common;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.commons.userdb.profile.Address;
import org.sipfoundry.commons.userdb.profile.UserProfile;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.branch.BranchManager;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.commserver.imdb.ImdbTestCase;
import org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class CoreContextImplTestIntegration extends IntegrationTestCase {
    private static final int NUM_USERS = 10;
    private PermissionManager m_permissionManager;
    private CoreContext m_coreContext;
    private BranchManager m_branchManager;
    private SettingDao m_settingDao;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        sql("commserver/SeedLocations.sql");
        sql("domain/DomainSeed.sql");
    }
    public void testLoadByUserName() throws Exception {
        sql("common/UserSearchSeed.sql");
        assertNotNull(m_coreContext.loadUserByUserName("userseed5"));
        assertNull(m_coreContext.loadUserByUserName("wont find this guy"));
    }

    public void testLoadByAlias() throws Exception {
        sql("common/UserSearchSeed.sql");

        User user = m_coreContext.loadUserByAlias("2");
        assertNotNull(user);
        assertEquals("userseed2", user.getUserName());
        user = m_coreContext.loadUserByAlias("two");
        assertNotNull(user);
        assertEquals("userseed2", user.getUserName());
        user = m_coreContext.loadUserByAlias("5");
        assertNotNull(user);
        assertEquals("userseed5", user.getUserName());
        assertNull(m_coreContext.loadUserByAlias("666"));
        assertNull(m_coreContext.loadUserByAlias("userseed2"));
    }

    public void testLoadUserByConfiguredImId() throws Exception {
        sql("common/UserSearchSeed.sql");
        User user = m_coreContext.loadUserByUserName("userseed1");
        UserProfile profile = new UserProfile();
        profile.setUserId("1");
        profile.setImId("jagr");
        user.setUserProfile(profile);
        m_coreContext.saveUser(user);
        commit();

        User userIm = m_coreContext.loadUserByConfiguredImId("jagr");
        assertEquals("userseed1", userIm.getUserName());

        User user1 = m_coreContext.loadUserByConfiguredImId("test");
        assertNull(user1);
    }

    public void testLoadUserByUserNameOrAlias() throws Exception {
        sql("common/UserSearchSeed.sql");

        User user = m_coreContext.loadUserByUserNameOrAlias("2");
        assertNotNull(user);
        assertEquals("userseed2", user.getUserName());

        user = m_coreContext.loadUserByUserNameOrAlias("two");
        assertNotNull(user);
        assertEquals("userseed2", user.getUserName());

        user = m_coreContext.loadUserByUserNameOrAlias("5");
        assertNotNull(user);
        assertEquals("userseed5", user.getUserName());

        assertNull(m_coreContext.loadUserByUserNameOrAlias("666"));

        user = m_coreContext.loadUserByUserNameOrAlias("userseed2");
        assertNotNull(user);
        assertEquals("userseed2", user.getUserName());

        // There was a bug earlier where only users who had aliases could be loaded.
        // Test that a user without aliases can be loaded.
        user = m_coreContext.loadUserByUserNameOrAlias("userwithnoaliases");
        assertNotNull(user);
        assertEquals("userwithnoaliases", user.getUserName());
    }

    public void testCheckForDuplicateNameOrAlias() throws Exception {
        sql("common/UserSearchSeed.sql");
        final String UNIQUE_NAME = "uniqueNameThatDoesntExistInTheUniverseOrBeyond";

        // Check that a user with a unique name won't collide with any existing users
        User user = m_coreContext.newUser();
        user.setUserName(UNIQUE_NAME);
        assertNull(m_coreContext.checkForDuplicateNameOrAlias(user));

        // Check that a user with a duplicate name is found to be a duplicate
        user.setUserName("userseed4");
        assertEquals("userseed4", m_coreContext.checkForDuplicateNameOrAlias(user));

        // Check that a user with an alias that matches an existing username
        // is found to be a duplicate
        user.setUserName(UNIQUE_NAME);
        user.setAliasesString("userseed6");
        assertEquals("userseed6", m_coreContext.checkForDuplicateNameOrAlias(user));

        // Check that a user with an alias that matches an existing alias
        // is found to be a duplicate
        user.setAliasesString("two");
        assertEquals("two", m_coreContext.checkForDuplicateNameOrAlias(user));

        // Check that duplicate checking doesn't get confused by multiple collisions
        // with both usernames and aliases
        user.setUserName("userseed4");
        user.setAliasesString("two,1");
        assertNotNull(m_coreContext.checkForDuplicateNameOrAlias(user));

        // Check that collisions internal to the user are caught.
        // Note that duplicates within the aliases list are ignored.
        final String WASAWUSU = "wasawusu";
        user.setUserName(WASAWUSU);
        user.setAliasesString(WASAWUSU);
        assertEquals(WASAWUSU, m_coreContext.checkForDuplicateNameOrAlias(user));
        user.setUserName(UNIQUE_NAME);
        user.setAliasesString(WASAWUSU + "," + WASAWUSU);
        assertNull(m_coreContext.checkForDuplicateNameOrAlias(user));
    }

    public void testSearchByUserName() throws Exception {
        sql("common/UserSearchSeed.sql");

        User template = m_coreContext.newUser();;
        template.setUserName("userseed");
        List users = m_coreContext.loadUserByTemplateUser(template);

        assertEquals(7, users.size());
    }

    public void testSearchByAlias() throws Exception {
        sql("common/UserSearchSeed.sql");

        User template = m_coreContext.newUser();;
        template.setUserName("two");
        List users = m_coreContext.loadUserByTemplateUser(template);
        assertEquals(1, users.size());
        User user = (User) users.get(0);
        assertEquals("userseed2", user.getUserName());
    }

    public void testSearchByFirstName() throws Exception {
        sql("common/UserSearchSeed.sql");

        User template = m_coreContext.newUser();;
        template.setFirstName("user4");
        List users = m_coreContext.loadUserByTemplateUser(template);
        assertEquals(1, users.size());
        User user = (User) users.get(0);
        assertEquals("userseed4", user.getUserName());
    }

    public void testSearchByLastName() throws Exception {
        sql("common/UserSearchSeed.sql");

        User template = m_coreContext.newUser();;
        template.setLastName("seed5");
        List users = m_coreContext.loadUserByTemplateUser(template);
        assertEquals(1, users.size());
        User user = (User) users.get(0);
        assertEquals("userseed5", user.getUserName());
    }

    // Test that we are doing a logical "AND" not "OR". There is a user with
    // the specified first name, and a different user with the specified last
    // name. Because of the "AND" we should get no matches.
    public void testSearchByFirstAndLastName() throws Exception {
        sql("common/UserSearchSeed.sql");

        User template = m_coreContext.newUser();;
        template.setFirstName("user4");
        template.setLastName("seed5");
        List users = m_coreContext.loadUserByTemplateUser(template);
        assertEquals(0, users.size());
    }

    public void testSearchFormBlank() throws Exception {
        sql("common/UserSearchSeed.sql");

        User template = m_coreContext.newUser();
        template.setFirstName("");
        List users = m_coreContext.loadUserByTemplateUser(template);

        assertEquals(NUM_USERS, users.size());
    }

    @SuppressWarnings("deprecation")
    public void testLoadUsers() throws Exception {
        sql("common/UserSearchSeed.sql");

        List users = m_coreContext.loadUsers();

        // Check that we have the expected number of users
        assertEquals(NUM_USERS, users.size());
    }

    public void testDeleteUsers() throws Exception {
        sql("common/UserSearchSeed.sql");

        // Check that we have the expected number of users
        assertEquals(11, countRowsInTable("users"));

        // Delete two users
        List<Integer> usersToDelete = new ArrayList<Integer>();
        usersToDelete.add(1001);
        usersToDelete.add(1002);
        m_coreContext.deleteUsers(usersToDelete);
        commit();

        // We should have reduced the user count by two
        assertEquals(9, countRowsInTable("users"));
    }

    public void testDeleteUsersByUserName() throws Exception {
        sql("common/UserSearchSeed.sql");

        // Check that we have the expected number of users
        assertEquals(11, countRowsInTable("users"));

        // Delete two users
        List<String> usersToDelete = new ArrayList<String>();
        usersToDelete.add("userseed1");
        usersToDelete.add("userseed2");
        m_coreContext.deleteUsersByUserName(usersToDelete);
        commit();

        // We should have reduced the user count by two
        assertEquals(9, countRowsInTable("users"));
    }

    public void testLoadGroups() throws Exception {
        sql("common/UserGroupSeed.sql");
        List groups = m_coreContext.getGroups();
        assertEquals(1, groups.size());
        Group group = (Group) groups.get(0);
        assertEquals("SeedUserGroup1", group.getName());
    }

    public void testGetGroupByName() throws Exception {
        sql("common/UserGroupSeed.sql");

        Group g1 = m_coreContext.getGroupByName("SeedUserGroup1", false);
        assertNotNull(g1);
        assertEquals("SeedUserGroup1", g1.getName());

        Group g2 = m_coreContext.getGroupByName("bongo", false);
        assertNull(g2);
        assertEquals(1, countRowsInTable("group_storage"));

        g2 = m_coreContext.getGroupByName("bongo", true);
        assertNotNull(g2);
        assertEquals("bongo", g2.getName());
        commit();
        assertEquals(2, countRowsInTable("group_storage"));
    }

    public void testClear() throws Exception {
        sql("common/TestUserSeed.sql");
        m_coreContext.clear();
        commit();
        assertEquals(0, countRowsInTable("users"));
    }

    public void testCreateAdminGroupAndInitialUserTask() throws Exception {
        m_coreContext.createAdminGroupAndInitialUserTask();
        flush();
        User admin = m_coreContext.loadUserByUserName(User.SUPERADMIN);
        Group adminGroup = admin.getGroups().iterator().next();
        
        assertEquals(0, countRowsInTable("address_book_entry"));
        assertEquals(1, countRowsInTable("users"));
        assertEquals(1, countRowsInTable("group_storage"));
        assertEquals(2, countRowsInTable("setting_value"));
        Map<String, Object> u = db().queryForMap("select * from users");
        assertEquals("superadmin", u.get("user_name"));
        assertEquals("ENABLE", db().queryForObject("select value from setting_value where path = 'permission/application/superadmin'", String.class));
        assertEquals("DISABLE", db().queryForObject("select value from setting_value where path = 'permission/application/tui-change-pin'", String.class));

        // make sure that it works even if superadmin has its rights revoked
        admin.setPermission(PermissionName.SUPERADMIN, false);
        m_coreContext.saveUser(admin);
        assertFalse(admin.isAdmin());
        m_coreContext.createAdminGroupAndInitialUserTask();

        admin = m_coreContext.loadUserByUserName(User.SUPERADMIN);
        assertTrue(admin.isAdmin());
    }

    public void testCountUsers() throws Exception {
        sql("common/UserCountSeed.sql");
        assertEquals(4, m_coreContext.getUsersCount());
        assertEquals(7, m_coreContext.getAllUsersCount());
        assertEquals(3, m_coreContext.loadInternalUsers().size());
    }

    public void testCountUsersInGroup() throws Exception {
        sql("common/UserSearchSeed.sql");
        assertEquals(2, m_coreContext.getUsersInGroupCount(1001));
        assertEquals(3, m_coreContext.getUsersInGroupCount(1002));
    }

    public void testCountUsersInGroupWithSearch() throws Exception {
        sql("common/SampleUsersSeed.sql");
        assertEquals(1, m_coreContext.getUsersInGroupWithSearchCount(1001, "pha"));
        assertEquals(1, m_coreContext.getUsersInGroupWithSearchCount(1002, "add"));
        assertEquals(2, m_coreContext.getUsersInGroupWithSearchCount(1003, "l"));
    }

    public void testLoadUserPage() throws Exception {
        sql("common/SampleUsersSeed.sql");
        Collection page = m_coreContext.loadUsersByPage(null, null, null, 0, 2, "userName", true);
        assertEquals(2, page.size());
        User u = (User) page.iterator().next();
        assertEquals("alpha", u.getUserName());

        Collection next = m_coreContext.loadUsersByPage(null, null, null, 2, 2, "userName", true);
        assertEquals(2, next.size());
        User nextUser = (User) next.iterator().next();
        assertEquals("charlie", nextUser.getUserName());
    }

    public void testLoadUserPageDescending() throws Exception {
        sql("common/SampleUsersSeed.sql");
        // expect third user from bottom
        Collection page = m_coreContext.loadUsersByPage(null, null, null, 2, 2, "userName", false);
        User u = (User) page.iterator().next();
        assertEquals("horatio", u.getUserName());
    }

    public void testLoadUserPageOrderByFirstName() throws Exception {
        sql("common/SampleUsersSeed.sql");
        // try sorting by last name
        Collection page = m_coreContext.loadUsersByPage(null, null, null, 2, 2, "firstName", true);
        User u = (User) page.iterator().next();
        assertEquals("kyle", u.getUserName());
    }

    public void testLoadUserPageWithGroup() throws Exception {
        sql("common/SampleUsersSeed.sql");
        Collection page = m_coreContext.loadUsersByPage(null, 1001, null, 0, 10, "userName", true);
        assertEquals(1, page.size());
        User u = (User) page.iterator().next();
        assertEquals("alpha", u.getUserName());
    }

    public void testLoadUserPageWithUserSearch() throws Exception {
        sql("common/SampleUsersSeed.sql");
        commit();
        Collection<User> page = m_coreContext.loadUsersByPage("og", null, null, 0, 10, "userName", true);
        Iterator<User> users = page.iterator();
        assertEquals("alpha", users.next().getUserName());
        assertEquals("elephant", users.next().getUserName());
        assertEquals("frank", users.next().getUserName());
        assertEquals("gogo", users.next().getUserName());
        assertEquals("kyle", users.next().getUserName());
        assertFalse(users.hasNext());

        page = m_coreContext.loadUsersByPage("og", null, null, 2, 2, "userName", true);
        assertEquals(2, page.size());
        User u = (User) page.iterator().next();
        assertEquals("frank", u.getUserName());

        page = m_coreContext.loadUsersByPage("og", null, null, 4, 2, "userName", true);
        assertEquals(1, page.size());
        u = (User) page.iterator().next();
        assertEquals("kyle", u.getUserName());

        page = m_coreContext.loadUsersByPage("og", null, null, 5, 2, "userName", true);
        assertEquals(0, page.size());
    }

    public void testLoadUserPageWithUserSearchAndGroup() throws Exception {
        sql("common/SampleUsersSeed.sql");
        Collection page = m_coreContext.loadUsersByPage("og", 1003, null, 0, 10, "userName", true);
        assertEquals(1, page.size());
        User u = (User) page.iterator().next();
        assertEquals("gogo", u.getUserName());
    }

    public void testLoadUserPageWithUserSearchCaseInsensitive() throws Exception {
        sql("common/SampleUsersSeed.sql");
        Collection page = m_coreContext.loadUsersByPage("mamba", null, null, 0, 10, "userName", true);
        assertEquals(1, page.size());
        User u = (User) page.iterator().next();
        assertEquals("kyle", u.getUserName());
    }

    public void testIsAliasInUse() throws Exception {
        sql("common/SampleUsersSeed.sql");
        assertTrue(m_coreContext.isAliasInUse("janus")); // a user ID
        assertTrue(m_coreContext.isAliasInUse("dweezil")); // a user alias
        assertFalse(m_coreContext.isAliasInUse("jessica")); // a first name
    }

    public void testGetBeanIdsOfObjectsWithAlias() throws Exception {
        sql("common/SampleUsersSeed.sql");
        assertTrue(m_coreContext.getBeanIdsOfObjectsWithAlias("janus").size() == 1); // a user ID
        assertTrue(m_coreContext.getBeanIdsOfObjectsWithAlias("dweezil").size() == 1); // a user alias
        assertTrue(m_coreContext.getBeanIdsOfObjectsWithAlias("jessica").size() == 0); // a first name
    }

    public void testGetGroupSupervisors() throws Exception {
        sql("common/GroupSupervisorSeed.sql");
        Group group = m_settingDao.getGroup(1001);
        List<User> supervisors = m_coreContext.getGroupSupervisors(group);
        assertEquals(1, supervisors.size());
        assertEquals((Integer) 1001, supervisors.get(0).getId());
    }

    public void testUsersThatISupervise() throws Exception {
        sql("common/UsersThatISupervise.sql");
        User supervisor = m_coreContext.loadUser(2001);

        List<User> peons = m_coreContext.getUsersThatISupervise(supervisor);
        assertEquals(3, peons.size());
        assertEquals("peon1", peons.get(0).getUserName());
        assertEquals("peon2", peons.get(1).getUserName());
        assertEquals("peon5", peons.get(2).getUserName());
    }

    public void testCheckForValidExtensions() throws Exception {
        sql("common/UserSearchSeed.sql");

        //PermissionManagerImpl pm = new PermissionManagerImpl();
        //m_permissionManager.setModelFilesContext(TestHelper.getModelFilesContext());

        User user = m_coreContext.loadUserByAlias("2");
        assertNotNull(user);
        user.setPermissionManager(m_permissionManager);
        user.setPermission(PermissionName.VOICEMAIL, false);
        m_coreContext.saveUser(user);

        user = m_coreContext.loadUserByAlias("3");
        assertNotNull(user);
        user.setPermissionManager(m_permissionManager);
        user.setPermission(PermissionName.VOICEMAIL, true);
        m_coreContext.saveUser(user);

        // An blank lists should have no invalid exceptions
        Set<String> empty = Collections.emptySet();
        m_coreContext.checkForValidExtensions(empty, PermissionName.VOICEMAIL);

        // User with extension "10" doesn't exist
        try {
            m_coreContext.checkForValidExtensions(Arrays.asList("10"), PermissionName.VOICEMAIL);
            fail();
        } catch (UserException e) {
            assertTrue(true);
        }

        // User with extension "2" exist, but doesn't have voicemail permission
        try {
            m_coreContext.checkForValidExtensions(Arrays.asList("2"), PermissionName.VOICEMAIL);
            fail();
        } catch (UserException e) {
            assertTrue(true);
        }

        // User with extension "3" and voicemail permission;
        m_coreContext.checkForValidExtensions(Arrays.asList("3"), PermissionName.VOICEMAIL);
        try {
            m_coreContext.checkForValidExtensions(Arrays.asList("2", "3", "10"), PermissionName.VOICEMAIL);
            fail();
        } catch (UserException e) {
            assertTrue(e.getMessage().contains("2"));
            assertTrue(e.getMessage().contains("10"));
            assertFalse(e.getMessage().contains("3"));
        }
    }

    public void testGetSpecialUsers() {
        m_coreContext.initializeSpecialUsers();

        User user = m_coreContext.getSpecialUser(SpecialUserType.MEDIA_SERVER);
        assertEquals("~~id~media", user.getName());
        assertTrue(user.getSipPassword().length() >= 10);
        user = m_coreContext.getSpecialUser(SpecialUserType.PARK_SERVER);
        assertEquals("~~id~park", user.getName());
        assertTrue(user.getSipPassword().length() >= 10);
        user = m_coreContext.getSpecialUser(SpecialUserType.REGISTRAR_SERVER);
        assertEquals("~~id~registrar", user.getName());
        assertTrue(user.getSipPassword().length() >= 10);
    }

    public void testGetSharedUsers() throws Exception {
        sql("common/SharedUsersSeed.sql");
        List<User> sharedUsers = m_coreContext.getSharedUsers();
        assertEquals(1, sharedUsers.size());
        assertEquals((Integer) 1001, sharedUsers.get(0).getId());
    }

    public void testSaveAddressBook() throws Exception {
        sql("common/UserSearchSeed.sql");

        User user = m_coreContext.loadUser(1001);
        assertNotNull(user);

        UserProfile addressBook = user.getUserProfile();
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

        user.setUserProfile(addressBook);
        m_coreContext.saveUser(user);

        user = m_coreContext.loadUser(1001);
        assertNotNull(user);
        assertEquals("Museum of Science", user.getUserProfile().getCompanyName());
        assertEquals("(617) 723-2500", user.getUserProfile().getCellPhoneNumber());
        assertEquals("Data Entry Assistant", user.getUserProfile().getJobTitle());
        assertEquals("Boston", user.getUserProfile().getOfficeAddress().getCity());
        assertEquals("1 Science Park", user.getUserProfile().getOfficeAddress().getStreet());
    }

    public void testIsImIdUnique() throws Exception {
        loadDataSet("common/users-im-ids.db.xml");
        // check im id uniqueness for a new user
        initUsersImIdsSeed();
        User user = new User();
        user.setUserName("testUser");
        user.setUniqueId();
        assertTrue("ImId unique when no IM ID configured", m_coreContext.isImIdUnique(user));
        user.setImId("openfire1");
        assertFalse(m_coreContext.isImIdUnique(user));
        user.setImId("OpenFire1");
        assertFalse(m_coreContext.isImIdUnique(user));
        user.setImId("userseed3");
        assertFalse(m_coreContext.isImIdUnique(user));
        user.setImId("alias1");
        assertFalse(m_coreContext.isImIdUnique(user));
        user.setImId("openfire22");
        assertTrue(m_coreContext.isImIdUnique(user));
        // check im id uniqueness for an existing user
        User existingUser = m_coreContext.loadUser(1001);
        assertTrue(m_coreContext.isImIdUnique(existingUser));
    }

    private void initUsersImIdsSeed() {
     // prepare user profiles
        User userseed1 = m_coreContext.loadUserByUserName("userseed1");
        userseed1.setImId("openfire1");
        getUserProfileService().saveUserProfile(userseed1.getUserProfile());
        User userseed2 = m_coreContext.loadUserByUserName("userseed2");
        userseed2.setImId("openfire2");
        getUserProfileService().saveUserProfile(userseed2.getUserProfile());
        User userseed3 = m_coreContext.loadUserByUserName("userseed3");
        getUserProfileService().saveUserProfile(userseed3.getUserProfile());
        User user201 = m_coreContext.loadUserByUserName("201");
        getUserProfileService().saveUserProfile(user201.getUserProfile());
    }

    public void testImIdAsAlias() throws Exception {
        loadDataSet("common/users-im-ids.db.xml");
        initUsersImIdsSeed();
        // test if ImId as alias in use
        assertTrue(m_coreContext.isAliasInUse("openfire2"));

        // check im id uniqueness for a new user
        User user = new User();
        user.setImId("Alias1");
        assertEquals("Alias1", m_coreContext.checkForDuplicateNameOrAlias(user));
        assertEquals(1, m_coreContext.getBeanIdsOfObjectsWithAlias("openfire1").size());
    }

    public void testLoadUsersByPage() throws Exception {
        // there are 10 users in this file... starting from 1001 to 1010
        sql("common/UserSearchSeed.sql");

        List<User> users = m_coreContext.loadUsersByPage(0, 3);
        assertEquals(3, users.size());
        assertEquals(1001, users.get(0).getId().intValue());
        assertEquals(1003, users.get(2).getId().intValue());

        users = m_coreContext.loadUsersByPage(1, 250);
        assertEquals(9, users.size());
        assertEquals(1002, users.get(0).getId().intValue());
        assertEquals(1010, users.get(8).getId().intValue());

        users = m_coreContext.loadUsersByPage(4, 2);
        assertEquals(2, users.size());
        assertEquals(1005, users.get(0).getId().intValue());
        assertEquals(1006, users.get(1).getId().intValue());

        users = m_coreContext.loadUsersByPage(10, 0);
        assertEquals(0, users.size());

        users = m_coreContext.loadUsersByPage(10, 250);
        assertEquals(0, users.size());

        users = m_coreContext.loadUsersByPage(9, 1);
        assertEquals(1, users.size());
    }

    public void testAvailableGroups() {
        loadDataSet("common/UserGroupAvailable.db.xml");
        User user = m_coreContext.loadUser(1001);
        List<Group> groups = m_coreContext.getAvailableGroups(user);
        assertEquals(3, groups.size());
        for (Group group : groups) {
            assertTrue(group.getId() == null || group.getId() == 1001 || group.getId() == 1002
                    || group.getId() == 1004);
        }
    }

    public void testImAdminUsers() {
        loadDataSet("common/UserImAdmin.db.xml");
        List<User> users = m_coreContext.loadUserByAdmin();
        assertTrue(users.size() == 1);
        User user = users.get(0);
        assertEquals("123", user.getUserName());
    }

    public void testInheritedBranch() {
        loadDataSet("common/UserGroupAvailable.db.xml");
        User user = m_coreContext.loadUser(1001);
        Branch branch = m_branchManager.getBranch(101);
        user.setBranch(branch);
        try {
            m_coreContext.saveUser(user);
            fail();
        } catch (UserException ex) {

        }
        assertEquals(100, user.getInheritedBranch().getId().intValue());
    }

    public void testTrimUserValues() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        loadDataSetXml("commserver/seedLocations.xml");
        User user = m_coreContext.newUser();

        user.setFirstName("First  ");
        user.setLastName("  Last");
        user.setUserName(" username ");
        user.setImId(" imID ");
        user.setImDisplayName("displayName  ");
        user.addAlias(" Alias1");
        user.addAlias("Alias2 ");
        assertTrue(m_coreContext.saveUser(user));
        user = m_coreContext.loadUserByUserName("username");
        assertEquals("First", user.getFirstName());
        assertEquals("Last", user.getLastName());
        assertEquals("username", user.getUserName());
        assertEquals("imID", user.getImId());
        assertEquals("displayName", user.getImDisplayName());
        assertEquals("Alias1 Alias2", user.getAliasesString());
    }

    public void testAddRemoveGroup() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        loadDataSetXml("commserver/seedLocations.xml");
        Branch b1 = new Branch();
        b1.setName("b1");
        m_branchManager.saveBranch(b1);
        Branch b2 = new Branch();
        b2.setName("b2");
        m_branchManager.saveBranch(b2);

        Group g1 = new Group();
        g1.setResource("user");
        g1.setName("group1");
        g1.setBranch(b1);
        m_settingDao.saveGroup(g1);
        Group g2 = new Group();
        g2.setResource("user");
        g2.setName("group2");
        g2.setBranch(b2);
        m_settingDao.saveGroup(g2);

        User u1 = m_coreContext.newUser();
        u1.setUserName("u1");
        // u1.setBranch(b1);
        m_coreContext.saveUser(u1);

        m_coreContext.addToGroup(m_settingDao.getGroupByName("user", "group1").getId(),
                Arrays.asList(m_coreContext.loadUserByUserName("u1").getId()));
        m_coreContext.removeFromGroup(m_settingDao.getGroupByName("user", "group1").getId(),
                Arrays.asList(m_coreContext.loadUserByUserName("u1").getId()));

        u1.setBranch(b2);
        m_coreContext.saveUser(u1);
        try {
            m_coreContext.addToGroup(m_settingDao.getGroupByName("user", "group1").getId(),
                    Arrays.asList(m_coreContext.loadUserByUserName("u1").getId()));
            fail();
        } catch (UserException e) {
            assertEquals("u1", e.getRawParams()[0]);
            assertEquals("b2", e.getRawParams()[1]);
            assertEquals("b1", e.getRawParams()[2]);
        }
        
        User u3 = m_coreContext.newUser();
        u3.setUserName("u3");
        m_coreContext.saveUser(u3);
        
        m_coreContext.addToGroup(m_settingDao.getGroupByName("user", "group1").getId(),
                Arrays.asList(m_coreContext.loadUserByUserName("u3").getId()));
        try {
            m_coreContext.addToGroup(m_settingDao.getGroupByName("user", "group2").getId(),
                    Arrays.asList(m_coreContext.loadUserByUserName("u3").getId()));
            fail();
        } catch (UserException e) {
            assertEquals("u3", e.getRawParams()[0]);
            assertEquals("b1", e.getRawParams()[1]);
            assertEquals("b2", e.getRawParams()[2]);
        }
    }

    public void testSaveUserWithUnameFax () throws Exception {
        loadDataSet("common/user_with_fax.db.xml");
        // test if ImId as alias in use
        assertTrue(m_coreContext.isAliasInUse("400"));
        assertTrue(m_coreContext.isAliasInUse("500"));
    }

    public void testGetReplicables() {
        //note that users are not returned into this List, they are
        //replicated usgin a different mechanism
        //only groups and special users are in the list
        Group g1 = new Group();
        g1.setResource("user");
        g1.setName("group1");
        m_settingDao.saveGroup(g1);
        m_coreContext.createAdminGroupAndInitialUser("1234");
        List<Replicable> replicables = m_coreContext.getReplicables();
        assertTrue(replicables.contains(g1));
        assertTrue(replicables.size() == 11); //9 special users, 2 groups
    }

    public void setBranchManager(BranchManager branchManager) {
        m_branchManager = branchManager;
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    public void setPermissionManager(PermissionManager permissionManager) {
        m_permissionManager = permissionManager;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
