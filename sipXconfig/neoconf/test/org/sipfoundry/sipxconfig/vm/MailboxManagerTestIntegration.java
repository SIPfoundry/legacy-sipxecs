/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import java.io.File;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.common.AbstractUser;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendant;

public class MailboxManagerTestIntegration extends IntegrationTestCase {
    private LocalMailboxManagerImpl m_mailboxManager;
    private SettingDao m_settingDao;

    private CoreContext m_coreContext;

    public void testLoadPersonalAttendantPerUser() throws Exception {
        loadDataSetXml("admin/dialplan/sbc/domain.xml");
        loadDataSetXml("admin/commserver/seedLocations.xml");
        assertEquals(0, countRowsInTable("personal_attendant"));

        User newUser = m_coreContext.newUser();
        m_coreContext.saveUser(newUser);

        PersonalAttendant pa = m_mailboxManager.loadPersonalAttendantForUser(newUser);
        assertNotNull(pa);
        flush();
        assertEquals(1, countRowsInTable("personal_attendant"));
        assertSame(newUser, pa.getUser());

        m_mailboxManager.storePersonalAttendant(pa);

        flush();
        assertEquals(1, countRowsInTable("personal_attendant"));

        m_coreContext.deleteUser(newUser);

        flush();
        assertEquals(0, countRowsInTable("personal_attendant"));
    }

    public void testDeleteUserMailbox() throws Exception {
        loadDataSetXml("admin/dialplan/sbc/domain.xml");
        loadDataSetXml("admin/commserver/seedLocations.xml");
        User newUser = m_coreContext.newUser();
        newUser.setUserName("200");
        m_coreContext.saveUser(newUser);

        File mailstore = MailboxManagerTest.createTestMailStore();
        m_mailboxManager.setMailstoreDirectory(mailstore.getAbsolutePath());
        LocalMailbox mbox = ((LocalMailboxManagerImpl) m_mailboxManager).getMailbox("200");
        assertTrue(mbox.getUserDirectory().exists());

        PersonalAttendant pa = m_mailboxManager.loadPersonalAttendantForUser(newUser);
        flush();
        m_mailboxManager.storePersonalAttendant(pa);
        flush();

        m_coreContext.deleteUser(newUser);

        flush();
        assertFalse(((LocalMailbox) mbox).getUserDirectory().exists());
    }

    public void testUserGroupOperator() throws Exception {
        loadDataSetXml("admin/dialplan/sbc/domain.xml");
        loadDataSetXml("admin/commserver/seedLocations.xml");
        
        User user = m_coreContext.newUser();
        user.setUserName("200");
        m_coreContext.saveUser(user);
        assertEquals(null, user.getOperator());
        
        Group g = new Group();
        g.setName("group");
        g.setSettingValue(AbstractUser.OPERATOR_SETTING, "111");
        m_settingDao.saveGroup(g);
        user.addGroup(g);
        m_coreContext.saveUser(user);
        assertEquals("111", user.getOperator());
        
        g.setSettingValue(AbstractUser.OPERATOR_SETTING, "");
        m_settingDao.saveGroup(g);
        assertEquals(null, user.getOperator());
        
        user.setOperator("123");
        m_coreContext.saveUser(user);
        assertEquals("123", user.getOperator());
    }
    
    public void setMailboxManagerImpl(LocalMailboxManagerImpl mailboxManager) {
        m_mailboxManager = mailboxManager;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }
}
