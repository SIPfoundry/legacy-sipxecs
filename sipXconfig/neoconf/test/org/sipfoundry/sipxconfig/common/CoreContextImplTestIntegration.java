/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.common;

import java.util.Arrays;
import java.util.List;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.branch.BranchManager;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

/**
 * Contains Integration tests. All tests from CoreContextImplTestDb should be moved here and
 * CoreContextImplTestDb should be deleted
 */
public class CoreContextImplTestIntegration extends IntegrationTestCase {
    private CoreContext m_coreContext;
    private BranchManager m_branchManager;
    private SettingDao m_settingDao;

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void testIsImIdUnique() throws Exception {
        loadDataSet("common/users-im-ids.db.xml");
        // check im id uniqueness for a new user
        User user = new User();
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

    public void testImIdAsAlias() throws Exception {
        loadDataSet("common/users-im-ids.db.xml");
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
        loadDataSet("common/UserSearchSeed.xml");

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
        loadDataSetXml("admin/commserver/seedLocations.xml");
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
        loadDataSetXml("admin/commserver/seedLocations.xml");
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
    }

    public void testSaveUserWithUnameFax () throws Exception {
        loadDataSet("common/user_with_fax.db.xml");
        // test if ImId as alias in use
        assertTrue(m_coreContext.isAliasInUse("400"));
        assertTrue(m_coreContext.isAliasInUse("500"));
    }
    
    public void setBranchManager(BranchManager branchManager) {
        m_branchManager = branchManager;
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

}
