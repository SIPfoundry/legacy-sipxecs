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

import java.util.Collection;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.context.ApplicationContext;

public class UserSearchManagerImplTestDb extends TestCase {

    private CoreContext m_coreContext;
    private IdentityToBean m_identityToBean;
    private UserSearchManager m_userSearch;

    protected void setUp() throws Exception {
        ApplicationContext context = TestHelper.getApplicationContext();
        m_userSearch = (UserSearchManager) context.getBean(UserSearchManager.CONTEXT_BEAN_NAME);
        m_coreContext = (CoreContext) context.getBean(CoreContext.CONTEXT_BEAN_NAME);
        m_identityToBean = new IdentityToBean(m_coreContext);
        TestHelper.cleanInsert("ClearDb.xml");
        IndexManager indexManager = (IndexManager) context
                .getBean(IndexManager.CONTEXT_BEAN_NAME);
        indexManager.indexAll();
    }


    public void testSearchEmpty() throws Exception {
        User user = new User();
        user.setFirstName("first");
        user.setLastName("last");
        user.setUserName("bongo");

        m_coreContext.saveUser(user);

        User template = new User();
        Collection collection = m_userSearch.search(template, 0, -1, m_identityToBean);
        assertEquals(1, collection.size());
        assertTrue(collection.contains(user));
    }

    public void testSearchFirstName() throws Exception {
        User user = new User();
        user.setFirstName("first");
        user.setLastName("last");
        user.setUserName("bongo");

        m_coreContext.saveUser(user);

        User template = new User();

        template.setFirstName("first");
        Collection collection = m_userSearch.search(template, 0, -1, m_identityToBean);
        assertEquals(1, collection.size());
        assertTrue(collection.contains(user));

        template = new User();
        template.setUserName("bon");
        collection = m_userSearch.search(template, 0, -1, m_identityToBean);
        assertEquals(1, collection.size());
        assertTrue(collection.contains(user));

        template = new User();
        template.setUserName("bOn");
        collection = m_userSearch.search(template, 0, -1, m_identityToBean);
        assertEquals(1, collection.size());
        assertTrue(collection.contains(user));

        template.setUserName("");
        template.setFirstName("bongo");

        collection = m_userSearch.search(template, 0, -1, m_identityToBean);
        assertEquals(0, collection.size());

    }

    public void testAliases() {
        User user = new User();
        user.setFirstName("First");
        user.setLastName("Last");
        user.setUserName("bongo");

        user.setAliasesString("aaA, bcd");
        m_coreContext.saveUser(user);

        User template = new User();
        template.setUserName("aaa");
        Collection collection = m_userSearch.search(template, 0, -1, m_identityToBean);
        assertEquals(1, collection.size());
        assertTrue(collection.contains(user));

        template = new User();
        template.setUserName("aaa");
        template.setLastName("kuku");
        collection = m_userSearch.search(template, 0, -1, m_identityToBean);
        assertEquals(0, collection.size());
    }
}
