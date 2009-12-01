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
import java.util.Set;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.im.ExternalImAccount;

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

    public void testSaveUserExternalImAccount() throws Exception {
        loadDataSet("common/UserSearchSeed.xml");

        User user = m_coreContext.loadUser(1001);
        assertNotNull(user);

        Set<ExternalImAccount> externalImAccounts = user.getExternalImAccounts();
        assertNotNull(externalImAccounts);
        assertEquals(0, externalImAccounts.size());

        ExternalImAccount yahooAccount = new ExternalImAccount();
        yahooAccount.setUser(user);
        yahooAccount.setType("yahoo");
        yahooAccount.setUsername("yahoo_username");
        yahooAccount.setPassword("yahoo_password");
        yahooAccount.setEnabled(false);
        externalImAccounts.add(yahooAccount);

        ExternalImAccount googletalkAccount = new ExternalImAccount();
        googletalkAccount.setUser(user);
        googletalkAccount.setType("gtalk");
        googletalkAccount.setUsername("google_username");
        googletalkAccount.setPassword("google_password");
        googletalkAccount.setDisplayName("google_nickname");
        externalImAccounts.add(googletalkAccount);

        user.setExternalImAccounts(externalImAccounts);
        m_coreContext.saveUser(user);
        externalImAccounts = user.getExternalImAccounts();
        assertNotNull(externalImAccounts);
        assertEquals(2, externalImAccounts.size());
    }

    public void testGetExternalAccountById() throws Exception {
        loadDataSet("common/ExternalImAccountSeed.db.xml");

        ExternalImAccount account = m_coreContext.getExternalAccountById(101);
        assertNotNull(account);
        assertEquals(true, account.isEnabled());
        assertEquals("gtalk", account.getType());
        assertEquals("john.doe", account.getUsername());
        assertEquals("John", account.getDisplayName());

        User user = account.getUser();
        assertNotNull(user);
        assertEquals(2, user.getExternalImAccounts().size());
    }

    public void testSaveExternalImAccount() throws Exception {
        loadDataSet("common/ExternalImAccountSeed.db.xml");

        ExternalImAccount account = m_coreContext.getExternalAccountById(101);
        User user = account.getUser();
        assertNotNull(user);
        Set<ExternalImAccount> externalImAccounts = user.getExternalImAccounts();
        assertNotNull(externalImAccounts);
        assertEquals(2, externalImAccounts.size());

        ExternalImAccount newAccount = new ExternalImAccount();
        newAccount.setUser(user);
        newAccount.setEnabled(true);
        newAccount.setType("icq");
        newAccount.setUsername("john");
        newAccount.setPassword("123");
        m_coreContext.saveExternalAccount(newAccount);

        externalImAccounts = user.getExternalImAccounts();
        assertNotNull(externalImAccounts);
        assertEquals(2, externalImAccounts.size());
    }
}
