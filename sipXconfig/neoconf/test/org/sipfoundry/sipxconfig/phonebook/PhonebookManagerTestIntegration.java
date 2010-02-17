/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.phonebook;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public class PhonebookManagerTestIntegration extends IntegrationTestCase {
    private PhonebookManager m_phonebookManager;
    private SettingDao m_settingDao;
    private CoreContext m_coreContext;

    public void setPhonebookManager(PhonebookManager phonebookManager) {
        m_phonebookManager = phonebookManager;
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void testGetPhonebook() throws Exception {
        loadDataSet("phonebook/PhonebookSeed.db.xml");
        Phonebook p = m_phonebookManager.getPhonebook(1001);
        assertEquals(1, p.getMembers().size());
    }

    public void testSavePhonebook() throws Exception {
        Phonebook p = new Phonebook();
        p.setName("test-save");
        m_phonebookManager.savePhonebook(p);
    }

    public void testUpdatePhonebookWithMemberAndConsumerGroups() throws Exception {
        loadDataSet("phonebook/PhonebookSeed.db.xml");
        Phonebook p = m_phonebookManager.getPhonebook(1001);
        p.setName("update-with-groups-test");

        List<Group> groups = m_settingDao.getGroupsByString(User.GROUP_RESOURCE_ID, "phonebook-users", true);
        p.replaceMembers(groups);
        p.replaceConsumers(groups);
        m_phonebookManager.savePhonebook(p);

        Phonebook updatedPhonebook = m_phonebookManager.getPhonebook(1001);

        assertEquals(1, updatedPhonebook.getMembers().size());
        assertEquals(1, updatedPhonebook.getConsumers().size());
    }

    public void testPhoneBooksByUser() throws Exception {
        // yellowthroat should not see any sparrows, but see other warblers and ducks
        loadDataSet("phonebook/PhonebookMembersAndConsumersSeed.db.xml");
        User yellowthroat = m_coreContext.loadUser(1001);
        Collection<Phonebook> books = m_phonebookManager.getPublicPhonebooksByUser(yellowthroat);
        Iterator<PhonebookEntry> entries = m_phonebookManager.getEntries(books, yellowthroat).iterator();
        assertEquals("canadian", entries.next().getNumber());
        assertEquals("mallard", entries.next().getNumber());
        assertEquals("pintail", entries.next().getNumber());
        assertEquals("yellowthroat", entries.next().getNumber());
        assertFalse(entries.hasNext());
    }

    public void testAllPhoneBooksByUser() {
        loadDataSet("phonebook/PhonebookSeed.db.xml");
        User user1003 = m_coreContext.loadUser(1003);
        Collection<Phonebook> phonebooks = m_phonebookManager.getAllPhonebooksByUser(user1003);
        assertEquals(2, phonebooks.size());

        User user1002 = m_coreContext.loadUser(1002);
        phonebooks = m_phonebookManager.getAllPhonebooksByUser(user1002);
        assertEquals(1, phonebooks.size());

        User user1001 = m_coreContext.loadUser(1001);
        phonebooks = m_phonebookManager.getAllPhonebooksByUser(user1001);
        assertEquals(0, phonebooks.size());

        User user1000 = m_coreContext.loadUser(1000);
        phonebooks = m_phonebookManager.getAllPhonebooksByUser(user1000);
        assertEquals(1, phonebooks.size());

    }

    public void testUpdateOnGroupDelete() throws Exception {
        loadDataSet("phonebook/PhonebookMembersAndConsumersSeed.db.xml");
        Group g = m_coreContext.getGroupByName("warblers", false);
        assertNotNull(g);
        m_settingDao.deleteGroups(Collections.singleton(g.getId()));

        Phonebook phonebook1 = m_phonebookManager.getPhonebook(1001);
        Phonebook phonebook2 = m_phonebookManager.getPhonebook(1003);

        assertEquals(0, phonebook1.getMembers().size());
        assertEquals(0, phonebook1.getConsumers().size());

        assertEquals(1, phonebook2.getMembers().size());
        assertEquals(0, phonebook2.getConsumers().size());

    }

    public void testFileUploadPhonebookEntries() throws Exception {
        loadDataSet("phonebook/PhonebookFileEntriesSeed.db.xml");

        Phonebook p = m_phonebookManager.getPhonebook(new Integer(2001));

        // testing Gmail CSV file import
        m_phonebookManager.addEntriesFromFile(2001, getClass().getResourceAsStream("phonebook_gmail.csv"));

        // testing Outlook CSV file import
        m_phonebookManager.addEntriesFromFile(2001, getClass().getResourceAsStream("phonebook_outlook.csv"));

        // testing CSV file import
        m_phonebookManager.addEntriesFromFile(2001, getClass().getResourceAsStream("phonebook.csv"));

        // testing vCard file import
        m_phonebookManager.addEntriesFromFile(2001, getClass().getResourceAsStream("phonebook.vcf"));

        Collection<PhonebookEntry> entries = m_phonebookManager.getEntries(p);
        Iterator<PhonebookEntry> it = entries.iterator();

        assertEquals(4, entries.size());
        PhonebookEntry entry1 = it.next();
        assertEquals("Abe", entry1.getFirstName());
        assertEquals("Lincoln", entry1.getLastName());
        assertEquals("12345", entry1.getNumber());

        PhonebookEntry entry2 = it.next();
        assertEquals("Zack", entry2.getFirstName());
        assertEquals("McCracken", entry2.getLastName());
        assertEquals("66667", entry2.getNumber());

        PhonebookEntry entry3 = it.next();
        assertEquals("William", entry3.getFirstName());
        assertEquals("Riker", entry3.getLastName());
        assertEquals("1234", entry3.getNumber());

        PhonebookEntry entry4 = it.next();
        assertEquals("John", entry4.getFirstName());
        assertEquals("Wayne", entry4.getLastName());
        assertEquals("5555", entry4.getNumber());
    }

    public void testDeletePhonebooks() throws Exception {
        loadDataSet("phonebook/PhonebookFileEntriesSeed.db.xml");
        Collection<Phonebook> booksBeforeDelete = m_phonebookManager.getPhonebooks();
        assertEquals(2, booksBeforeDelete.size());

        m_phonebookManager.deletePhonebooks(Arrays.asList(2001, 2002));
        Collection<Phonebook> books = m_phonebookManager.getPhonebooks();
        assertEquals(0, books.size());
    }

    public void testDeletePhonebookWithEntries() throws Exception {
        loadDataSet("phonebook/PhonebookFileEntriesSeed.db.xml");
        Collection<Phonebook> booksBeforeDelete = m_phonebookManager.getPhonebooks();
        assertEquals(2, booksBeforeDelete.size());

        Phonebook book = m_phonebookManager.getPhonebook(new Integer(2001));
        m_phonebookManager.addEntriesFromFile(2001, getClass().getResourceAsStream("phonebook.csv"));
        Collection<PhonebookEntry> entries = m_phonebookManager.getEntries(book);
        assertEquals(1, entries.size());

        m_phonebookManager.deletePhonebooks(Arrays.asList(2001));
        Collection<Phonebook> books = m_phonebookManager.getPhonebooks();
        assertEquals(1, books.size());
    }

    public void testGetPrivatePhonebook() throws Exception {
        loadDataSet("phonebook/PhonebookSeed.db.xml");
        User portaluser = m_coreContext.loadUser(1002);

        Phonebook privatePhonebook = m_phonebookManager.getPrivatePhonebook(portaluser);
        assertNotNull(privatePhonebook);
        assertEquals("privatePhonebook_1002", privatePhonebook.getName());
        assertEquals(1, m_phonebookManager.getEntries(Collections.singletonList(privatePhonebook), portaluser)
                .size());

        User anotheruser = m_coreContext.loadUser(1003);

        Collection<Phonebook> phonebooks = m_phonebookManager.getPublicPhonebooksByUser(anotheruser);
        assertEquals(2, m_phonebookManager.getPagedPhonebook(phonebooks, anotheruser, "0", "10", null).getEntries()
                .size());

    }

    public void testDeletePrivatePhonebook() throws Exception {
        loadDataSet("phonebook/PhonebookSeed.db.xml");
        // should not return private phonebooks - set back to 4 when supported
        assertEquals(2, m_phonebookManager.getPhonebooks().size());

        User portaluser = m_coreContext.loadUser(1002);
        Phonebook privatePhonebook = m_phonebookManager.getPrivatePhonebook(portaluser);
        assertEquals("privatePhonebook_1002", privatePhonebook.getName());
        m_coreContext.deleteUser(portaluser);

        assertNull(m_phonebookManager.getPrivatePhonebook(portaluser));
    }

    public void testGetPrivatePhonebookCreateIfRequired() throws Exception {
        loadDataSet("phonebook/PhonebookSeed.db.xml");
        // should not return private phonebooks - set back to 3 when supported
        User portaluser = m_coreContext.loadUser(1002);
        Phonebook portalUserPrivatePhonebook = m_phonebookManager.getPrivatePhonebookCreateIfRequired(portaluser);
        assertEquals("privatePhonebook_1002", portalUserPrivatePhonebook.getName());

        User yellowthroat = m_coreContext.loadUser(1001);
        Phonebook yellowthroatPrivatePhonebook = m_phonebookManager.getPrivatePhonebookCreateIfRequired(yellowthroat);
        assertEquals("privatePhonebook_1001", yellowthroatPrivatePhonebook.getName());
    }

    public void testGetPagedPhonebook() throws Exception {
        loadDataSet("phonebook/PhonebookMembersAndConsumersSeed.db.xml");
        User yellowthroat = m_coreContext.loadUser(1001);
        Collection<Phonebook> books = m_phonebookManager.getPublicPhonebooksByUser(yellowthroat);

        PagedPhonebook pagedPhonebook = m_phonebookManager.getPagedPhonebook(books, yellowthroat, "0", "1", "l");
        assertEquals(4, pagedPhonebook.getSize());
        assertEquals(3, pagedPhonebook.getFilteredSize());
        assertEquals(0, pagedPhonebook.getStartRow());
        assertEquals(1, pagedPhonebook.getEndRow());
        Iterator<PhonebookEntry> entries = pagedPhonebook.getEntries().iterator();
        assertEquals("mallard", entries.next().getNumber());

        pagedPhonebook = m_phonebookManager.getPagedPhonebook(books, yellowthroat, "1", "3", "l");
        entries = pagedPhonebook.getEntries().iterator();
        assertEquals("pintail", entries.next().getNumber());
        assertEquals("yellowthroat", entries.next().getNumber());

        pagedPhonebook = m_phonebookManager.getPagedPhonebook(books, yellowthroat, "0", "3", null);
        assertEquals(4, pagedPhonebook.getSize());
        assertEquals(4, pagedPhonebook.getFilteredSize());
        assertEquals(0, pagedPhonebook.getStartRow());
        assertEquals(3, pagedPhonebook.getEndRow());
        entries = pagedPhonebook.getEntries().iterator();
        assertEquals("canadian", entries.next().getNumber());
        assertEquals("mallard", entries.next().getNumber());
        assertEquals("pintail", entries.next().getNumber());

        pagedPhonebook = m_phonebookManager.getPagedPhonebook(books, yellowthroat, "0", "10", null);
        assertEquals(4, pagedPhonebook.getSize());
        assertEquals(4, pagedPhonebook.getFilteredSize());
        assertEquals(0, pagedPhonebook.getStartRow());
        assertEquals(4, pagedPhonebook.getEndRow());

        pagedPhonebook = m_phonebookManager.getPagedPhonebook(books, yellowthroat, "a", "b", null);
        assertEquals(4, pagedPhonebook.getSize());
        assertEquals(4, pagedPhonebook.getFilteredSize());
        assertEquals(0, pagedPhonebook.getStartRow());
        assertEquals(4, pagedPhonebook.getEndRow());
    }

    public void testGetPrivatePagedPhonebook() throws Exception {
        loadDataSet("phonebook/PhonebookMembersAndConsumersSeed.db.xml");
        User canadian = m_coreContext.loadUser(1002);
        Collection<Phonebook> books = m_phonebookManager.getPublicPhonebooksByUser(canadian);

        PagedPhonebook pagedPhonebook = m_phonebookManager.getPagedPhonebook(books, canadian, "0", "100", null);
        assertEquals(5, pagedPhonebook.getSize());
        Iterator<PhonebookEntry> entries = pagedPhonebook.getEntries().iterator();
        PhonebookEntry contact1 = entries.next();
        assertEquals("canadian", contact1.getNumber());
        assertEquals(new Integer(-1), contact1.getId());
        PhonebookEntry contact2 = entries.next();
        assertEquals("mallard", contact2.getNumber());
        assertEquals(new Integer(-1), contact2.getId());
        assertEquals("pintail", entries.next().getNumber());
        assertEquals("yellowthroat", entries.next().getNumber());
        PhonebookEntry editableContact = entries.next();
        assertEquals(new Integer(101), editableContact.getId());
        assertEquals("10020", editableContact.getNumber());
    }
}
