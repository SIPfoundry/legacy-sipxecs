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

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;

public class PagingContextImplTestIntegration extends IntegrationTestCase {
    private PagingContext m_pagingContext;
    private CoreContext m_coreContext;

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setPagingContext(PagingContext pagingContext) {
        m_pagingContext = pagingContext;
    }

    protected void onSetUpInTransaction() throws Exception {
        loadDataSet("paging/paging.db.xml");
        loadDataSet("paging/user-paging.db.xml");
    }

    public void testGetPagingPrefix() throws Exception {
        assertEquals("*77", m_pagingContext.getPagingPrefix());
    }

    public void testGetSipTraceLevel() throws Exception {
        assertEquals("NONE", m_pagingContext.getSipTraceLevel());
    }

    public void testGetPagingGroups() throws Exception {
        List<PagingGroup> groups = m_pagingContext.getPagingGroups();
        assertEquals(3, groups.size());
        PagingGroup group1 = groups.get(0);
        assertEquals(111, group1.getPageGroupNumber());
        assertEquals("Sales", group1.getDescription());
        assertEquals(true, group1.isEnabled());
        assertEquals("TadaTada.wav", group1.getSound());
        assertEquals(60, group1.getTimeout());
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
        assertEquals(112, group.getPageGroupNumber());
        assertEquals("Engineering", group.getDescription());
        assertEquals(false, group.isEnabled());
        assertEquals("TadaTada.wav", group.getSound());
        assertEquals(60, group.getTimeout());
        Set<User> users = group.getUsers();
        assertEquals(1, users.size());
    }

    public void testDeletePagingGroupsById() throws Exception {
        List<PagingGroup> groups = m_pagingContext.getPagingGroups();
        assertEquals(3, groups.size());
        List<Integer> groupsIds = new ArrayList<Integer>();
        groupsIds.add(101);
        groupsIds.add(102);
        m_pagingContext.deletePagingGroupsById(groupsIds);
        groups = m_pagingContext.getPagingGroups();

        // 2 paging groups should disappear
        assertEquals(1, groups.size());
    }

    public void testSavePagingPrefix() throws Exception {
        assertEquals("*77", m_pagingContext.getPagingPrefix());
        m_pagingContext.setPagingPrefix("*88");
        assertEquals("*88", m_pagingContext.getPagingPrefix());
    }

    public void testSaveSipTraceLevel() throws Exception {
        assertEquals("NONE", m_pagingContext.getSipTraceLevel());
        m_pagingContext.setSipTraceLevel("DEBUG");
        assertEquals("DEBUG", m_pagingContext.getSipTraceLevel());
    }

    public void testSavePagingGroup() throws Exception {
        List<PagingGroup> groups = m_pagingContext.getPagingGroups();
        assertEquals(3, groups.size());

        PagingGroup group = new PagingGroup();
        group.setPageGroupNumber(114);
        group.setDescription("test");
        group.setSound("TadaTada.wav");
        group.setTimeout(120);
        Set<User> users = new HashSet<User>();
        users.add(m_coreContext.loadUser(1003));
        group.setUsers(users);
        m_pagingContext.savePagingGroup(group);
        groups = m_pagingContext.getPagingGroups();
        assertEquals(4, groups.size());
    }

    public void testSaveDuplicateNumberPagingGroup() throws Exception {
        PagingGroup groupWithDuplicateNumber = new PagingGroup();
        groupWithDuplicateNumber.setPageGroupNumber(111);
        try {
            m_pagingContext.savePagingGroup(groupWithDuplicateNumber);
        } catch (UserException ex) {
            assertTrue(true);
        }
    }

    public void testClear() throws Exception {
        m_pagingContext.clear();

        // they should be gone
        List<PagingGroup> groups = m_pagingContext.getPagingGroups();
        assertEquals(0, groups.size());
    }
}
