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

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.lang.StringUtils;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManagerImpl.CsvFileFormatError;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManagerImpl.PhoneEntryComparator;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManagerImpl.StringArrayPhonebookEntry;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.test.PhonebookTestHelper;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class PhonebookManagerTest extends TestCase {

    public void testMkdirs() {
        String dir = TestHelper.getTestDirectory() + File.separator + System.currentTimeMillis();
        assertFalse(new File(dir).exists());
        PhonebookManagerImpl.mkdirs(dir);
        assertTrue(new File(dir).exists());
    }

    public void testGetEmptyPhonebookRows() {
        PhonebookManagerImpl context = new PhonebookManagerImpl();
        assertEquals(0, context.getEntries(new Phonebook()).size());
    }

    public void testGetRows() {
        Phonebook phonebook = new Phonebook();
        Group group = new Group();
        User user = new User();
        user.setFirstName("Tweety");
        user.setLastName("Bird");
        user.setUserName("tbird");
        phonebook.setMembers(Collections.singleton(group));
        Collection<User> users = Collections.singleton(user);
        IMocksControl coreContextControl = EasyMock.createControl();
        CoreContext coreContext = coreContextControl.createMock(CoreContext.class);
        coreContext.getGroupMembers(group);
        coreContextControl.andReturn(users);
        coreContextControl.replay();

        PhonebookManagerImpl context = new PhonebookManagerImpl();
        context.setCoreContext(coreContext);
        Collection<PhonebookEntry> entries = context.getEntries(phonebook);
        assertEquals(1, entries.size());
        PhonebookEntry entry = entries.iterator().next();
        assertEquals("Tweety", entry.getFirstName());

        coreContextControl.verify();
    }

    public void testPhoneEntryComparator() {
        PhoneEntryComparator c = new PhoneEntryComparator();
        IMocksControl phonebookEntryControl = EasyMock.createControl();
        PhonebookEntry a = phonebookEntryControl.createMock(PhonebookEntry.class);
        PhonebookEntry b = phonebookEntryControl.createMock(PhonebookEntry.class);

        a.getLastName();
        phonebookEntryControl.andReturn("Avocet");
        b.getLastName();
        phonebookEntryControl.andReturn("Vireo");
        phonebookEntryControl.replay();

        assertTrue(c.compare(a, b) < 0);

        phonebookEntryControl.reset();
        a.getLastName();
        phonebookEntryControl.andReturn("Avocet");
        a.getFirstName();
        phonebookEntryControl.andReturn("Southern");
        b.getLastName();
        phonebookEntryControl.andReturn("Avocet");
        b.getFirstName();
        phonebookEntryControl.andReturn("Northern");
        phonebookEntryControl.replay();

        assertTrue(c.compare(a, b) > 0);

        phonebookEntryControl.reset();
        a.getLastName();
        phonebookEntryControl.andReturn("Avocet");
        a.getFirstName();
        phonebookEntryControl.andReturn("Northern");
        a.getNumber();
        phonebookEntryControl.andReturn("1234");

        b.getLastName();
        phonebookEntryControl.andReturn("Avocet");
        b.getFirstName();
        phonebookEntryControl.andReturn("Northern");
        b.getNumber();
        phonebookEntryControl.andReturn("abc");
        phonebookEntryControl.replay();

        assertTrue(c.compare(a, b) < 0);

        phonebookEntryControl.verify();
    }

    public void testStringArrayPhonebookEntry() {
        try {
            new StringArrayPhonebookEntry(new String[2]);
            fail();
        } catch (CsvFileFormatError e) {
            assertTrue(true);
        }
    }

    public void testStringArrayPhonebookEntryOk() {
        new StringArrayPhonebookEntry(new String[3]);
    }

    public void testGetCsvFile() {
        PhonebookManagerImpl context = new PhonebookManagerImpl();
        String dir = TestUtil.getTestSourceDirectory(getClass());
        context.setExternalUsersDirectory(dir);

        Phonebook phonebook = new Phonebook();
        phonebook.setMembersCsvFilename("bogus.csv");
        try {
            context.getEntries(phonebook);
            fail();
        } catch (RuntimeException expected) {
            assertTrue(true);
        }

        phonebook.setMembersCsvFilename("phonebook.csv");
        Collection<PhonebookEntry> entries = context.getEntries(phonebook);
        assertEquals(1, entries.size());
    }

    public void testGetVCardFile() {
        PhonebookManagerImpl context = new PhonebookManagerImpl();
        String dir = TestUtil.getTestSourceDirectory(getClass());
        context.setExternalUsersDirectory(dir);

        Phonebook phonebook = new Phonebook();
        phonebook.setMembersVcardFilename("bogus.vcf");
        try {
            context.getEntries(phonebook);
            fail();
        } catch (RuntimeException expected) {
            assertTrue(true);
        }

        phonebook.setMembersVcardFilename("phonebook.vcf");
        context.setTelType("work");
        Collection<PhonebookEntry> entries = context.getEntries(phonebook);
        assertEquals(1, entries.size());
    }

    public void testSearch() {
        final PhonebookTestHelper testHelper = new PhonebookTestHelper();

        CoreContext coreContextMock = EasyMock.createMock(CoreContext.class);
        testHelper.configureCoreContextMock(coreContextMock);
        EasyMock.replay(coreContextMock);

        // need to override the getEntries method to control what entries we use
        PhonebookManagerImpl out = new PhonebookManagerImpl() {
            public Collection<PhonebookEntry> getEntries(Collection<Phonebook> phonebooks) {
                return testHelper.getPhonebookEntries();
            }
        };
        out.setCoreContext(coreContextMock);

        List<PhonebookEntry> allEntries = new ArrayList(testHelper.getPhonebookEntries());
        // require there is at least one test user defined
        if (allEntries.size() < 1) {
            fail("Not enough test users defined by test helper");
        }

        PhonebookEntry testEntry = allEntries.get(0);
        String username = testHelper.getUserByUsername(testEntry.getNumber()).getUserName();

        // test searching by username in all lowercase
        Collection<PhonebookEntry> entriesThatMatchUsername = out.search(Collections
                .singletonList(new Phonebook()), username.toLowerCase());

        assertEquals(1, entriesThatMatchUsername.size());
        assertTrue(entriesThatMatchUsername.contains(testEntry));

        // test searching by username in all caps
        Collection<PhonebookEntry> entriesThatMatchUsernameCaps = out.search(Collections
                .singletonList(new Phonebook()), username.toUpperCase());

        assertEquals(1, entriesThatMatchUsernameCaps.size());
        assertTrue(entriesThatMatchUsernameCaps.contains(testEntry));

        // test searching by first name
        Collection<PhonebookEntry> entriesThatMatchFirstName = out.search(Collections
                .singletonList(new Phonebook()), testEntry.getFirstName());

        assertEquals(1, entriesThatMatchFirstName.size());
        assertTrue(entriesThatMatchFirstName.contains(testEntry));

        // test searching by last name - should return more than one result
        String lastName = testEntry.getLastName();
        int lastNameCount = 0;
        for (PhonebookEntry entry : testHelper.getPhonebookEntries()) {
            if (lastName.equals(entry.getLastName())) {
                lastNameCount++;
            }
        }
        Collection<PhonebookEntry> entriesThatMatchLastName = out.search(Collections
                .singletonList(new Phonebook()), testEntry.getLastName());

        assertEquals(lastNameCount, entriesThatMatchLastName.size());
        assertTrue(entriesThatMatchLastName.contains(testEntry));

        // test searching by partial first name
        Collection<PhonebookEntry> entriesThatMatchPartialFirstName = out.search(Collections
                .singletonList(new Phonebook()), testEntry.getFirstName().substring(0, 3));

        assertEquals(1, entriesThatMatchPartialFirstName.size());
        assertTrue(entriesThatMatchPartialFirstName.contains(testEntry));

        // test searching with alias
        User userWithAlias = null;
        for (User user : testHelper.getTestUsers()) {
            if (StringUtils.isNotEmpty(user.getAliasesString())) {
                userWithAlias = user;
                break;
            }
        }

        Collection<PhonebookEntry> entriesThatMatchAlias = out.search(Collections
                .singletonList(new Phonebook()), userWithAlias.getAliases().iterator().next());

        assertEquals(1, entriesThatMatchAlias.size());
        assertTrue(entriesThatMatchAlias.contains(testHelper.getEntryByNumber(userWithAlias
                .getUserName())));

        out.search(Collections.singletonList(new Phonebook()), "300");
        out.search(Collections.singletonList(new Phonebook()), "nulluser");

    }
}
