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

/**
 * Contains Integration tests. All tests from CoreContextImplTestDb should be moved here and
 * CoreContextImplTestDb should be deleted
 */
public class CoreContextImplTestIntegration extends IntegrationTestCase {
    private CoreContext m_coreContext;

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void testIsImIdUnique() throws Exception {
        loadDataSetXml("common/Users.xml");
        // check im id uniqueness for a new user
        User user = new User();
        user.setUniqueId();
        user.setImId("openfire1");
        assertFalse(m_coreContext.isImIdUnique(user));
        user.setImId("openfire22");
        assertTrue(m_coreContext.isImIdUnique(user));
        // check im id uniqueness for an existing user
        User existingUser = m_coreContext.loadUser(1001);
        assertTrue(m_coreContext.isImIdUnique(existingUser));
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
}
