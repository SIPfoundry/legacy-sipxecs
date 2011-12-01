/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.ImdbTestCase;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public class UserTestIntegration extends ImdbTestCase {
    private CoreContext m_coreContext;
    private SettingDao m_settingDao;
    private ValidUsers m_validUsers;

    public void testLoadUser() {
        loadDataSet("common/TestUserSeed.db.xml");
        int userId = 1000;
        User user = m_coreContext.loadUser(1000);
        assertEquals(userId, user.getPrimaryKey());
        assertEquals(userId, user.getId().intValue());
    }

    public void testAvailableGroup() {
        loadDataSet("setting/user-group-branch.xml");
        User user6 = m_coreContext.loadUser(1005);
        Group group1 = m_settingDao.loadGroup(1000);
        Group group2 = m_settingDao.loadGroup(1001);
        Group group3 = m_settingDao.loadGroup(1002);
        Group group4 = m_settingDao.loadGroup(1003);
        assertFalse(user6.isGroupAvailable(group1));
        assertTrue(user6.isGroupAvailable(group3));
        assertTrue(user6.isGroupAvailable(group4));
        User user2 = m_coreContext.loadUser(1001);
        user2.isGroupAvailable(group2);
    }

    public void testLastUpdated() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        loadDataSetXml("admin/commserver/seedLocationsAndServices.xml");
        Long past = System.currentTimeMillis();
        User u1 = m_coreContext.newUser();
        u1.setUserName("u1");
        m_coreContext.saveUser(u1);
        
        assertTrue(!m_validUsers.getUsersUpdatedAfter(past).isEmpty());
        
        Long now = System.currentTimeMillis();
        assertTrue(m_validUsers.getUsersUpdatedAfter(now).isEmpty());
        
        m_coreContext.saveUser(u1);
        assertTrue(!m_validUsers.getUsersUpdatedAfter(now).isEmpty());
        
    }
    
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    public void setValidUsers(ValidUsers validUsers) {
        m_validUsers = validUsers;
    }
}
