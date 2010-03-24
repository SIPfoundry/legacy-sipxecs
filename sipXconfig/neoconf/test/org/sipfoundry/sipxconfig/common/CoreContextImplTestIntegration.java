/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.common;

import java.util.List;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.branch.BranchManager;
import org.sipfoundry.sipxconfig.setting.Group;

/**
 * Contains Integration tests. All tests from CoreContextImplTestDb should be moved here and
 * CoreContextImplTestDb should be deleted
 */
public class CoreContextImplTestIntegration extends IntegrationTestCase {
    private CoreContext m_coreContext;
    private BranchManager m_branchManager;

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

        // strangely, 0 is ignored if there are still records to return
        users = m_coreContext.loadUsersByPage(9, 0);
        assertEquals(1, users.size());
    }

    public void testAvailableGroups() {
        loadDataSet("common/UserGroupAvailable.db.xml");
        User user = m_coreContext.loadUser(1001);
        List<Group> groups = m_coreContext.getAvailableGroups(user);
        assertEquals(3, groups.size());
        for (Group group : groups) {
            assertTrue(group.getId() == null || group.getId() == 1001 ||
                    group.getId() == 1002 || group.getId() == 1004);
        }
    }

    public void testInheritedBranch() {
        loadDataSet("common/UserGroupAvailable.db.xml");
        User user = m_coreContext.loadUser(1001);
        Branch branch = m_branchManager.getBranch(101);
        user.setBranch(branch);
        m_coreContext.saveUser(user);
        assertEquals(100, user.getBranch().getId().intValue());
    }

    public void testTrimUserValues() {
        User user = new User();
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

    public void setBranchManager(BranchManager branchManager) {
        m_branchManager = branchManager;
    }
}
