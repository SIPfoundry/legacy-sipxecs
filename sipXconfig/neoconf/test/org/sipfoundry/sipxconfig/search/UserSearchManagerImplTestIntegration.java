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

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.context.ApplicationContext;
import org.springframework.data.mongodb.core.MongoTemplate;

public class UserSearchManagerImplTestIntegration extends IntegrationTestCase {
    private CoreContext m_coreContext;
    private IdentityToBean m_identityToBean;
    private UserSearchManager m_userSearchManager;
    private IndexManager m_indexManager;

    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        sql("commserver/SeedLocations.sql");
        m_identityToBean = new IdentityToBean(m_coreContext);
        m_indexManager.indexAll();
    }

    public void testSearchEmpty() throws Exception {
        User user = m_coreContext.newUser();;
        user.setFirstName("first");
        user.setLastName("last");
        user.setUserName("bongo");

        m_coreContext.saveUser(user);

        User template =  m_coreContext.newUser();
        Collection collection = m_userSearchManager.search(template, 0, -1, m_identityToBean);
        assertEquals(1, collection.size());
        assertTrue(collection.contains(user));
    }

    public void testSearchFirstName() throws Exception {
        User user = m_coreContext.newUser();
        user.setFirstName("first");
        user.setLastName("last");
        user.setUserName("bongo");

        m_coreContext.saveUser(user);

        User template = m_coreContext.newUser();;

        template.setFirstName("first");
        Collection collection = m_userSearchManager.search(template, 0, -1, m_identityToBean);
        assertEquals(1, collection.size());
        assertTrue(collection.contains(user));

        template = m_coreContext.newUser();;
        template.setUserName("bon");
        collection = m_userSearchManager.search(template, 0, -1, m_identityToBean);
        assertEquals(1, collection.size());
        assertTrue(collection.contains(user));

        template = m_coreContext.newUser();;
        template.setUserName("bOn");
        collection = m_userSearchManager.search(template, 0, -1, m_identityToBean);
        assertEquals(1, collection.size());
        assertTrue(collection.contains(user));

        template.setUserName("");
        template.setFirstName("bongo");

        collection = m_userSearchManager.search(template, 0, -1, m_identityToBean);
        assertEquals(0, collection.size());

    }

    public void testAliases() {
        ((MongoTemplate) getUserProfileService()).dropCollection("userProfile");
        User user = m_coreContext.newUser();;
        user.setFirstName("First");
        user.setLastName("Last");
        user.setUserName("bongo");

        user.setAliasesString("aaA, bcd");
        m_coreContext.saveUser(user);

        User template = m_coreContext.newUser();
        template.setUserName("aaa");
        Collection collection = m_userSearchManager.search(template, 0, -1, m_identityToBean);
        assertEquals(1, collection.size());
        assertTrue(collection.contains(user));

        template = m_coreContext.newUser();;
        template.setUserName("aaa");
        template.setLastName("kuku");
        collection = m_userSearchManager.search(template, 0, -1, m_identityToBean);
        assertEquals(0, collection.size());
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setIndexManager(IndexManager indexManager) {
        m_indexManager = indexManager;
    }

    public void setUserSearchManager(UserSearchManager userSearchManager) {
        m_userSearchManager = userSearchManager;
    }
}
