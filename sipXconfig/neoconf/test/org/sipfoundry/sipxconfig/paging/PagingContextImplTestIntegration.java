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

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;

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
        super.onSetUpInTransaction();
        db().execute("select truncate_all()");
        sql("paging/paging.sql");
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

    public void testSaveCode() throws Exception {
        PagingSettings settings = new PagingSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        assertEquals("*77", settings.getPrefix());
        settings = m_pagingContext.getSettings();
        settings.setPrefix("8585");
        m_pagingContext.saveSettings(settings);
        assertEquals("8585", settings.getPrefix());
        // test null
        settings.setPrefix("");
        try {
            m_pagingContext.saveSettings(settings);
            fail();
        } catch (UserException e) {

        }
    }

    public void testInvalidCode() throws Exception {
        PagingSettings settings = new PagingSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        assertEquals("*77", settings.getPrefix());
        settings = m_pagingContext.getSettings();
        settings.setPrefix("%");
        try {
            m_pagingContext.saveSettings(settings);
            fail();
        } catch (UserException e) {

        }
        settings.setPrefix("\\");
        try {
            m_pagingContext.saveSettings(settings);
            fail();
        } catch (UserException e) {

        }
        settings.setPrefix("%43");
        try {
            m_pagingContext.saveSettings(settings);
        } catch (UserException e) {
            fail();
        }
        settings.setPrefix("-_.!~*'()=+$,;?a-zA-Z0-9");
        try {
            m_pagingContext.saveSettings(settings);
        } catch (UserException e) {
            fail();
        }
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
