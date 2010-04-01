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

import java.io.BufferedInputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import static java.util.Arrays.asList;
import static java.util.Collections.singleton;
import static java.util.Collections.singletonList;

import junit.framework.TestCase;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.bulk.csv.CsvParserImpl;
import org.sipfoundry.sipxconfig.bulk.vcard.VcardParserImpl;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManagerImpl.FileEntrySearchPredicate;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManagerImpl.GoogleEntrySearchPredicate;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManagerImpl.PhoneEntryComparator;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManagerImpl.PhonebookEntryPredicate;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManagerImpl.StringArrayPhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManagerImpl.UserPhonebookEntry;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.test.PhonebookTestHelper;

public class PhonebookManagerTest extends TestCase {

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
        phonebook.setMembers(singleton(group));
        Collection<User> users = singleton(user);
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
        PhonebookEntry a = new PhonebookEntry();
        PhonebookEntry b = new PhonebookEntry();

        a.setLastName("Avocet");
        b.setLastName("Vireo");

        assertTrue(c.compare(a, b) < 0);

        a.setFirstName("Southern");
        b.setLastName("Avocet");
        b.setFirstName("Northern");

        assertTrue(c.compare(a, b) > 0);

        a.setLastName("Avocet");
        a.setFirstName("Northern");
        a.setNumber("1234");

        b.setLastName("Avocet");
        b.setFirstName("Northern");
        b.setNumber("abc");

        assertTrue(c.compare(a, b) < 0);

    }

    public void testStringArrayPhonebookEntry() {
        try {
            new StringArrayPhonebookEntry(new String[2]);
            fail();
        } catch (UserException e) {
            assertTrue(true);
        }
    }

    public void testStringArrayPhonebookEntryOk() {
        // no exceptions expected
        new StringArrayPhonebookEntry(new String[3]);
    }

    public void testInvalidFile() throws Exception {
        PhonebookManagerImpl context = new PhonebookManagerImpl();
        VcardParserImpl impl = new VcardParserImpl();
        context.setCsvParser(new CsvParserImpl());
        context.setVcardParser(impl);
        context.setVcardEncoding("US-ASCII");

        try {
            // use invalid file format
            Phonebook phonebook2 = new Phonebook();
            context.addEntries(phonebook2, getClass().getResourceAsStream("PhonebookSeed.db.xml"));
            fail("Should fail");
        } catch (UserException e) {
            assertEquals("&msg.invalidPhonebookFormat", e.getMessage());
        }
    }

    public void testIsVcard() throws Exception {
        PhonebookManagerImpl context = new PhonebookManagerImpl();
        context.setVcardEncoding("UTF-8");

        assertTrue(context.isVcard(toInputStream("BEGIN:VCARD"), "UTF-8"));
        assertTrue(context.isVcard(toInputStream("\n\nBEGIN:VCARD"), "UTF-8"));
        assertFalse(context.isVcard(toInputStream("\nsomething\nBEGIN:VCARD"), "UTF-8"));
        assertFalse(context.isVcard(toInputStream("something"), "UTF-8"));
    }

    private static BufferedInputStream toInputStream(String text) throws Exception {
        return new BufferedInputStream(new ByteArrayInputStream(text.getBytes("UTF-8")));
    }

    public void testGetCsvFile() throws Exception {
        PhonebookManagerImpl context = new PhonebookManagerImpl();
        VcardParserImpl impl = new VcardParserImpl();
        CsvParserImpl csvParser = new CsvParserImpl();
        csvParser.setSkipHeaderLine(false);
        context.setCsvParser(csvParser);
        context.setVcardParser(impl);
        context.setVcardEncoding("US-ASCII");

        Phonebook phonebook1 = new Phonebook();
        context.addEntries(phonebook1, getClass().getResourceAsStream("phonebook.csv"));
        assertEquals(1, phonebook1.getEntries().size());
    }

    public void testGetVCardFile() throws Exception {
        PhonebookManagerImpl context = new PhonebookManagerImpl();
        CsvParserImpl csvParser = new CsvParserImpl();
        csvParser.setSkipHeaderLine(false);
        context.setCsvParser(csvParser);
        VcardParserImpl impl = new VcardParserImpl();
        context.setVcardParser(impl);
        context.setVcardEncoding("US-ASCII");

        Phonebook phonebook1 = new Phonebook();
        context.addEntries(phonebook1, getClass().getResourceAsStream("phonebook.vcf"));
        assertEquals(1, phonebook1.getEntries().size());
    }

    public void testParseContactUsingPhonebookEntry() {
        InputStream testPhonebookFile = getClass().getResourceAsStream("StevenSpielberg.vcf");
        Reader reader = new InputStreamReader(testPhonebookFile);
        VcardParserImpl parser = new VcardParserImpl();
        Map<String, PhonebookEntry> entries = new TreeMap<String, PhonebookEntry>();

        parser.parse(reader, new PhonebookManagerImpl.PhonebookEntryMaker(entries, false));
        List<PhonebookEntry> entriesList = new ArrayList(entries.values());

        assertEquals(1, entriesList.size());

        PhonebookEntry entry = entriesList.get(0);

        assertEquals("Steven", entry.getFirstName());
        assertEquals("Spielberg", entry.getLastName());
        assertEquals("999", entry.getNumber());

        AddressBookEntry abe = entry.getAddressBookEntry();
        assertEquals("9986", abe.getCellPhoneNumber());
        assertEquals("080332", abe.getHomePhoneNumber());
        assertEquals("080-45", abe.getFaxNumber());
        assertEquals("stevens@sipx.com", abe.getEmailAddress());
        assertEquals("stevens123@sipx.com", abe.getAlternateEmailAddress());
        assertEquals("Qantom", abe.getCompanyName());
        assertEquals("Test Engineer", abe.getJobTitle());
        assertEquals("SCS", abe.getJobDept());

        Address homeAddress = abe.getHomeAddress();
        assertEquals("3rd Phase JP Nagar", homeAddress.getStreet());
        assertEquals("560078", homeAddress.getZip());
        assertEquals("India", homeAddress.getCountry());
        assertEquals("Ktaka", homeAddress.getState());
        assertEquals("Bangalore", homeAddress.getCity());

        Address officeAddress = abe.getOfficeAddress();
        assertEquals("Andheri East", officeAddress.getStreet());
        assertEquals("400010", officeAddress.getZip());
        assertEquals("India", officeAddress.getCountry());
        assertEquals("Maharastra", officeAddress.getState());
        assertEquals("Mumbai", officeAddress.getCity());
        assertEquals("PostOffice", officeAddress.getOfficeDesignation());

    }

    public void testExportVCard() throws Exception {
        VcardWriter vcardWriter = new VcardWriter();
        PhonebookManagerImpl context = new PhonebookManagerImpl();
        context.setVcardWriter(vcardWriter);
        context.setVcardEncoding("US-ASCII");

        ByteArrayOutputStream empty = new ByteArrayOutputStream();
        context.exportPhonebook(new ArrayList(), empty);

        PhonebookEntry e1 = new StringArrayPhonebookEntry("Jean Luc", "Picard", "1234");
        PhonebookEntry e2 = new StringArrayPhonebookEntry("Luke", "Skywalker", "1235");
        PhonebookEntry e3 = new StringArrayPhonebookEntry("", "", "1235");
        PhonebookEntry e4 = new StringArrayPhonebookEntry("Frank", "Dawson", "+1-919-676-9515", "Senior Programmer",
                "IT Dept", "Lotus Development Corporation", "", "+34(345)112-345", "+34 (445) 43 22", "",
                "+1-919-676-9564", "", "", "", "Mountain View", "U.S.A.", "CA", "501 E. Middlefield Rd.", "94043",
                "Raleigh", "U.S.A.", "NC", "6544 Battleford Drive", "27613-3502", "Lotus PostOffice",
                "Frank_Dawson@Lotus.com", "fdawson@earthlink.net");

        ByteArrayOutputStream actual = new ByteArrayOutputStream();
        context.exportPhonebook(asList(e1, e2, e3, e4), actual);

        InputStream expectedStream = getClass().getResourceAsStream("export.test.vcf");
        assertNotNull(expectedStream);
        String expected = IOUtils.toString(expectedStream);
        assertEquals(expected, actual.toString("UTF-8"));
    }

    public void testSearch() {
        final PhonebookTestHelper testHelper = new PhonebookTestHelper();

        CoreContext coreContextMock = EasyMock.createMock(CoreContext.class);
        testHelper.configureCoreContextMock(coreContextMock);
        EasyMock.replay(coreContextMock);

        List<PhonebookEntry> allEntries = new ArrayList(testHelper.getPhonebookEntries());
        // require there is at least one test user defined
        if (allEntries.size() < 1) {
            fail("Not enough test users defined by test helper");
        }

        PhonebookEntry testEntry = allEntries.get(0);
        User userPortal = testHelper.getUserByUsername(testEntry.getNumber());
        String username = userPortal.getUserName();

        // need to override the getEntries method to control what entries we use
        PhonebookManagerImpl out = new PhonebookManagerImpl() {
            @Override
            public Collection<PhonebookEntry> getEntries(Collection<Phonebook> phonebooks, User user) {
                return testHelper.getPhonebookEntries();
            }
        };
        out.setCoreContext(coreContextMock);

        // test searching by username in all lowercase
        Collection<PhonebookEntry> entriesThatMatchUsername = out.search(singletonList(new Phonebook()), username
                .toLowerCase(), userPortal);

        assertEquals(1, entriesThatMatchUsername.size());
        assertTrue(entriesThatMatchUsername.contains(testEntry));

        // test searching by username in all caps
        Collection<PhonebookEntry> entriesThatMatchUsernameCaps = out.search(singletonList(new Phonebook()),
                username.toUpperCase(), userPortal);

        assertEquals(1, entriesThatMatchUsernameCaps.size());
        assertTrue(entriesThatMatchUsernameCaps.contains(testEntry));

        // test searching by first name
        Collection<PhonebookEntry> entriesThatMatchFirstName = out.search(singletonList(new Phonebook()), testEntry
                .getFirstName(), userPortal);

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
        Collection<PhonebookEntry> entriesThatMatchLastName = out.search(singletonList(new Phonebook()), testEntry
                .getLastName(), userPortal);

        assertEquals(lastNameCount, entriesThatMatchLastName.size());
        assertTrue(entriesThatMatchLastName.contains(testEntry));

        // test searching by partial first name
        Collection<PhonebookEntry> entriesThatMatchPartialFirstName = out.search(singletonList(new Phonebook()),
                testEntry.getFirstName().substring(0, 3), userPortal);

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

        Collection<PhonebookEntry> entriesThatMatchAlias = out.search(singletonList(new Phonebook()), userWithAlias
                .getAliases().iterator().next(), userPortal);

        assertEquals(1, entriesThatMatchAlias.size());
        assertTrue(entriesThatMatchAlias.contains(testHelper.getEntryByNumber(userWithAlias.getUserName())));

        out.search(singletonList(new Phonebook()), "300", userPortal);
        out.search(singletonList(new Phonebook()), "nulluser", userPortal);
    }

    public void testUserPhoneBookEntry() throws Exception {
        User user = new User();
        UserPhonebookEntry entry = new PhonebookManagerImpl.UserPhonebookEntry(user);

        user.setUserName("500");
        assertEquals("500", entry.getNumber());

        user.setUserName("abcd");
        assertEquals("abcd", entry.getNumber());

        user.setAliasesString("501");
        assertEquals("501", entry.getNumber());
    }

    public void testGetEntries() throws Exception {
        // check if get entries removes duplicates properly
        PhonebookManagerImpl impl = new PhonebookManagerImpl();

        PhonebookEntry entry = new PhonebookEntry();
        entry.setNumber("1234");
        entry.setFirstName("Adam");

        PhonebookEntry entry2 = new PhonebookEntry();
        entry2.setNumber("1234");
        entry2.setFirstName("Bob");

        Phonebook phonebook = new Phonebook();
        phonebook.setEntries(asList(entry, entry, entry2));

        Collection<PhonebookEntry> entries = impl.getEntries(phonebook);
        assertEquals(2, entries.size());
        Iterator<PhonebookEntry> it = entries.iterator();
        assertEquals(it.next().getFirstName(), "Adam");
        assertEquals(it.next().getFirstName(), "Bob");
    }

    public void testPhonebookEntryPredicate() {
        PhonebookEntry a = new PhonebookEntry();
        PhonebookEntry b = new PhonebookEntry();

        a.setLastName("a");
        a.setFirstName("b");
        a.setNumber("201");
        AddressBookEntry abe = new AddressBookEntry();
        abe.setEmailAddress("test@test.com");
        a.setAddressBookEntry(abe);

        PhonebookEntryPredicate predicate = new PhonebookEntryPredicate("q");
        assertFalse(predicate.evaluate(a));

        predicate = new PhonebookEntryPredicate("a");
        assertTrue(predicate.evaluate(a));

        predicate = new PhonebookEntryPredicate("b");
        assertTrue(predicate.evaluate(a));

        predicate = new PhonebookEntryPredicate("test");
        assertTrue(predicate.evaluate(a));
    }

    public void testEntrySearchPredicates() {
        PhonebookEntry a = new FilePhonebookEntry();
        a.setInternalId("internalId");
        PhonebookEntry b = new PhonebookEntry();
        b.setInternalId("internalId");
        GooglePhonebookEntry c = new GooglePhonebookEntry();
        c.setGoogleAccount("account1");
        GooglePhonebookEntry d = new GooglePhonebookEntry();
        d.setGoogleAccount("account1");
        GooglePhonebookEntry e = new GooglePhonebookEntry();
        e.setGoogleAccount("account2");

        Collection<PhonebookEntry> entries = new ArrayList<PhonebookEntry>();
        entries.add(a);
        entries.add(b);
        entries.add(c);
        entries.add(d);
        entries.add(e);

        assertEquals(1, CollectionUtils.select(entries, new FileEntrySearchPredicate("internalId")).size());
        assertEquals(2, CollectionUtils.select(entries, new GoogleEntrySearchPredicate("account1")).size());
    }
}
