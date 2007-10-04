/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phonebook;

import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.context.ApplicationContext;

public class PhonebookManagerTestDb extends TestHelper.TestCaseDb {
    private PhonebookManager m_context;
    private CoreContext m_coreContext;
    private SettingDao m_settingDao;
    private ApplicationContext m_appContext;

    protected void setUp() throws Exception {
        m_appContext = TestHelper.getApplicationContext();
        m_context = (PhonebookManager) m_appContext.getBean(PhonebookManager.CONTEXT_BEAN_NAME);
        m_settingDao = (SettingDao) m_appContext.getBean(SettingDao.CONTEXT_NAME);
        m_coreContext = (CoreContext) m_appContext.getBean(CoreContext.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");        
    }
    
    public void testGetPhonebook() throws Exception {
        TestHelper.insertFlat("phonebook/PhonebookSeed.db.xml");        
        Phonebook p2 = m_context.getPhonebook(1001);
        assertEquals(1, p2.getMembers().size());
    }
    
    public void testSavePhonebook() throws Exception {
        Phonebook p = new Phonebook();
        p.setName("test-save");
        m_context.savePhonebook(p);       
    }
    
    public void testUpdatePhonebookWithMemberAndConsumerGroups() throws Exception {
        Phonebook p = new Phonebook();
        p.setName("update-with-groups-test");
        List<Group> groups = m_settingDao.getGroupsByString(User.GROUP_RESOURCE_ID,
                "phonebook-users", true);
        p.replaceMembers(groups);
        p.replaceConsumers(groups);
        m_context.savePhonebook(p);
        assertEquals(1, TestHelper.getConnection().getRowCount("phonebook_member"));
        assertEquals(1, TestHelper.getConnection().getRowCount("phonebook_consumer"));
    }
    
    public void testPhoneBooksByUser() throws Exception {
        // yellowthroat should not see any sparrows, but see other warblers and ducks 
        TestHelper.insertFlat("phonebook/PhonebookMembersAndConsumersSeed.db.xml");
        User yellowthroat = m_coreContext.loadUser(1001);
        Collection<Phonebook> books = m_context.getPhonebooksByUser(yellowthroat);
        Iterator<PhonebookEntry> entries = m_context.getEntries(books).iterator();
        assertEquals("canadian", entries.next().getNumber());
        assertEquals("mallard", entries.next().getNumber());
        assertEquals("pintail", entries.next().getNumber());
        assertEquals("yellowthroat", entries.next().getNumber());
        assertFalse(entries.hasNext());
    }
    
    public void testUpdateOnGroupDelete() throws Exception {
        TestHelper.insertFlat("phonebook/PhonebookMembersAndConsumersSeed.db.xml");
        Group g = m_coreContext.getGroupByName("warblers", false);
        assertNotNull(g);
        m_settingDao.deleteGroups(Collections.singleton(g.getId()));
        assertEquals(1, TestHelper.getConnection().getRowCount("phonebook_member"));
        assertEquals(0, TestHelper.getConnection().getRowCount("phonebook_consumer"));        
    }
}
