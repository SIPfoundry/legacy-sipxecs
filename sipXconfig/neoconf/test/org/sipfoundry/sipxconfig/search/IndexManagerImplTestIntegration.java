/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.search;

import java.util.List;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.springframework.context.ApplicationContext;

public class IndexManagerImplTestIntegration extends IntegrationTestCase {
    private SearchManager m_searchManager;
    private IndexManager m_indexManager;
    private CoreContext m_coreContext;
    private IdentityToBean m_identityToBean;

    protected void onSetUpBeforeTransaction() throws Exception {
        m_identityToBean = new IdentityToBean(m_coreContext);
        clear();
    }

    public void testSearchByName() throws Exception {
        sql("common/UserSearchSeed.sql");
        m_indexManager.indexAll();
        List users = m_searchManager.search("u*2", m_identityToBean);
        assertEquals(1, users.size());
        User user = (User) users.get(0);
        assertNotNull(users.iterator().next());
        assertEquals("userseed2", user.getUserName());
    }

    public void testSearchByAlias() throws Exception {
        sql("common/UserSearchSeed.sql");
        m_indexManager.indexAll();

        List users = m_searchManager.search("two", m_identityToBean);
        assertEquals(1, users.size());
        User user = (User) users.get(0);
        assertEquals("userseed2", user.getUserName());
    }

    public void testSearchReferencedUser() throws Exception {
        sql("search/phone_user.sql");
        m_indexManager.indexAll();

        List users = m_searchManager.search("kuku", null);
        // check if user has been only indexed once
        assertEquals(1, users.size());
    }

    public void testSearchPhoneModelLabel() throws Exception {
        sql("search/phone_user.sql");
        m_indexManager.indexAll();

        List phones = m_searchManager.search("Test Phone", null);
        // check if phone is found using the model label
        assertEquals(1, phones.size());
    }

    public void setSearchManager(SearchManager searchManager) {
        m_searchManager = searchManager;
    }

    public void setIndexManager(IndexManager indexManager) {
        m_indexManager = indexManager;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
