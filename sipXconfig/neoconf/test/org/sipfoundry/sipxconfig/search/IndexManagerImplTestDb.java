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

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.TestHelper.TestCaseDb;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.context.ApplicationContext;

public class IndexManagerImplTestDb extends TestCaseDb {

    private SearchManager m_searchManager;
    private IndexManager m_indexManager;
    private IdentityToBean m_identityToBean;

    protected void setUp() throws Exception {
        ApplicationContext context = TestHelper.getApplicationContext();
        m_searchManager = (SearchManager) context.getBean(SearchManager.CONTEXT_BEAN_NAME);
        m_indexManager = (IndexManager) context.getBean(IndexManager.CONTEXT_BEAN_NAME);
        CoreContext coreContext = (CoreContext) context.getBean(CoreContext.CONTEXT_BEAN_NAME);
        m_identityToBean = new IdentityToBean(coreContext);
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testSearchByName() throws Exception {
        TestHelper.cleanInsertFlat("common/UserSearchSeed.xml");
        m_indexManager.indexAll();

        List users = m_searchManager.search("u*2", m_identityToBean);
        assertEquals(1, users.size());
        User user = (User) users.get(0);
        assertNotNull(users.iterator().next());
        assertEquals("userseed2", user.getUserName());
    }

    public void testSearchByAlias() throws Exception {
        TestHelper.cleanInsertFlat("common/UserSearchSeed.xml");
        m_indexManager.indexAll();

        List users = m_searchManager.search("two", m_identityToBean);
        assertEquals(1, users.size());
        User user = (User) users.get(0);
        assertEquals("userseed2", user.getUserName());
    }

    public void testSearchReferencedUser() throws Exception {
        TestHelper.cleanInsertFlat("search/phone_user.db.xml");
        m_indexManager.indexAll();

        List users = m_searchManager.search("kuku", null);
        // check if user has been only indexed once
        assertEquals(1, users.size());
    }

    public void testSearchPhoneModelLabel() throws Exception {
        TestHelper.cleanInsertFlat("search/phone_user.db.xml");
        m_indexManager.indexAll();

        List phones = m_searchManager.search("Test Phone", null);
        // check if phone is found using the model label
        assertEquals(1, phones.size());
    }

}
