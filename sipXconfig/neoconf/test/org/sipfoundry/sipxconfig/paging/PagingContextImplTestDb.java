/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.paging;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.springframework.context.ApplicationContext;

public class PagingContextImplTestDb extends SipxDatabaseTestCase {
    private PagingContext m_pagingContext;
    private CoreContext m_coreContext;
    private PagingConfiguration m_pagingConfig;

    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_pagingContext = (PagingContext) appContext.getBean(PagingContext.CONTEXT_BEAN_NAME);
        m_coreContext = (CoreContext) appContext.getBean(CoreContext.CONTEXT_BEAN_NAME);
        m_pagingConfig = (PagingConfiguration) appContext.getBean(PagingConfiguration.CONTEXT_BEAN_NAME);
        m_pagingConfig.setEtcDirectory(TestHelper.getTestDirectory());
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.insertFlat("paging/PagingGroupSeed.xml");
        TestHelper.insertFlat("paging/UserPagingGroupSeed.xml");
    }

    public void testGetPagingPrefix() throws Exception {
        assertEquals("*77", m_pagingContext.getPagingPrefix());
    }

    public void testGetPagingGroups() throws Exception {
        List<PagingGroup> groups = m_pagingContext.getPagingGroups();
        assertEquals(3, groups.size());
        PagingGroup group1 = groups.get(0);
        assertEquals(new Long(111), group1.getPageGroupNumber());
        assertEquals("Sales", group1.getDescription());
        assertEquals(true, group1.isEnabled());
        assertEquals("TadaTada.wav", group1.getSound());
        assertEquals("*77", group1.getPrefix());
        Set<User> users = group1.getUsers();
        assertEquals(2, users.size());
        Iterator it = users.iterator();
        List<String> usersName = new ArrayList<String>();
        while (it.hasNext()) {
            User user = (User) it.next();
            usersName.add(user.getName());
        }
        assertTrue(usersName.contains("user1"));
        assertTrue(usersName.contains("user2"));
    }

    public void testGetPagingGroupById() throws Exception {
        PagingGroup group = m_pagingContext.getPagingGroupById(new Integer(101));
        assertEquals(new Long(112), group.getPageGroupNumber());
        assertEquals("Engineering", group.getDescription());
        assertEquals(false, group.isEnabled());
        assertEquals("TadaTada.wav", group.getSound());
        assertEquals("*77", group.getPrefix());
        Set<User> users = group.getUsers();
        assertEquals(1, users.size());
    }

    public void testDeletePagingGroupsById() throws Exception {
        List<PagingGroup> groups = m_pagingContext.getPagingGroups();
        assertEquals(3, groups.size());
        List<Integer> groupsIds = new ArrayList<Integer>();
        groupsIds.add(new Integer(101));
        groupsIds.add(new Integer(102));
        m_pagingContext.deletePagingGroupsById(groupsIds);
        groups = m_pagingContext.getPagingGroups();

        // 2 paging groups should disappear
        assertEquals(1, groups.size());
    }

    public void testSavePagingPrefix() throws Exception {
        assertEquals("*77", m_pagingContext.getPagingPrefix());
        m_pagingContext.savePagingPrefix("*88");
        assertEquals("*88", m_pagingContext.getPagingPrefix());
    }

    public void testSavePagingGroup() throws Exception {
        List<PagingGroup> groups = m_pagingContext.getPagingGroups();
        assertEquals(3, groups.size());

        PagingGroup group = new PagingGroup();
        group.setPageGroupNumber(new Long(114));
        group.setDescription("test");
        group.setSound("TadaTada.wav");
        group.setPrefix("*88");
        group.setEnabled(true);
        Set<User> users = new HashSet<User>();
        users.add(m_coreContext.loadUser(new Integer(1003)));
        group.setUsers(users);
        m_pagingContext.savePagingGroup(group);
        groups = m_pagingContext.getPagingGroups();
        assertEquals(4, groups.size());
    }

    public void testClear() throws Exception {
        m_pagingContext.clear();

        // they should be gone
        List<PagingGroup> groups = m_pagingContext.getPagingGroups();
        assertEquals(0, groups.size());
    }
}
