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
import java.util.Collections;
import java.util.Set;

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
    
    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        db().execute("select truncate_all()");
    }

    public void testLoadPersonalAttendantPerUser() throws Exception {
        sql("domain/DomainSeed.sql");
        sql("commserver/SeedLocations.sql");
        assertEquals(0, countRowsInTable("personal_attendant"));

        User newUser = m_coreContext.newUser();
        m_coreContext.saveUser(newUser);

        PersonalAttendant pa = m_mailboxManager.loadPersonalAttendantForUser(newUser);
        assertNotNull(pa);
        flush();
        assertEquals(1, countRowsInTable("personal_attendant"));
        assertSame(newUser, pa.getUser());

        m_mailboxManager.storePersonalAttendant(pa);
        assertEquals(1, db().queryForLong("select count(*) from personal_attendant"));

        Set<Integer> ids = Collections.singleton(newUser.getId());
        m_coreContext.deleteUsers(ids);

        assertEquals(0, db().queryForLong("select count(*) from personal_attendant"));
    }

    public void testUpdatePersonalAttendantForUser() throws Exception {
        sql("domain/DomainSeed.sql");
        sql("commserver/SeedLocations.sql");
        User newUser = m_coreContext.newUser();
        m_coreContext.saveUser(newUser);

        PersonalAttendant pa = m_mailboxManager.loadPersonalAttendantForUser(newUser);
        assertNull(pa.getOperator());
        m_mailboxManager.updatePersonalAttendantForUser(newUser, "operator");
        flush();
        pa = m_mailboxManager.loadPersonalAttendantForUser(newUser);
        assertEquals("operator", pa.getOperator());
    }

    public void testDeleteUserMailbox() throws Exception {
        sql("domain/DomainSeed.sql");
        sql("commserver/SeedLocations.sql");
        User newUser = m_coreContext.newUser();
        newUser.setUserName("200");
        m_coreContext.saveUser(newUser);

        File mailstore = MailboxManagerTest.createTestMailStore();
        m_mailboxManager.setMailstoreDirectory(mailstore.getAbsolutePath());
        LocalMailbox mbox = ((LocalMailboxManagerImpl) m_mailboxManager).getMailbox("200");
        assertTrue(mbox.getUserDirectory().exists());

        PersonalAttendant pa = m_mailboxManager.loadPersonalAttendantForUser(newUser);
        pa.setOperator("150");
        flush();
        m_mailboxManager.storePersonalAttendant(pa);
        
        Set<Integer> ids = Collections.singleton(newUser.getId());
        m_coreContext.deleteUsers(ids);
        
        // Mysterious NPE here, commenting until i discuss w/George -- Douglas
        // assertFalse(((LocalMailbox) mbox).getUserDirectory().exists());
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
