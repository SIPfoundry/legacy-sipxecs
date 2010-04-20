/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.test;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;

public class PhonebookTestHelper {

    public  static final String LUCENE_STOP_WORD_USERNAME = "the";

    private static final String NUMBER_PATTERN = "\\d+";
    private static final String SECONDUSER_USERNAME = "300";
    private static final String SECONDUSER_FIRSTNAME = "second";
    private static final String FIRSTUSER_FIRSTNAME = "first";
    private static final String FIRSTUSER_USERNAME = "firstUser";
    private static final String NULLUSER_USERNAME = "nulluser";
    private static final String USER_LASTNAME = "user";
    private final Collection<User> m_usersInPhonebook;
    private final Collection<PhonebookEntry> m_phonebookEntries;

    public PhonebookTestHelper() {
        m_usersInPhonebook = new ArrayList<User>();
        m_usersInPhonebook.add(createUser(FIRSTUSER_USERNAME, FIRSTUSER_FIRSTNAME, USER_LASTNAME,
                "200 anotherUserName 501"));
        m_usersInPhonebook.add(createUser(SECONDUSER_USERNAME, SECONDUSER_FIRSTNAME, USER_LASTNAME, null));
        m_usersInPhonebook.add(createUser(NULLUSER_USERNAME, null, null, null));

        m_phonebookEntries = new ArrayList<PhonebookEntry>();
        m_phonebookEntries.add(createPhonebookEntry(FIRSTUSER_USERNAME, FIRSTUSER_FIRSTNAME, USER_LASTNAME));
        m_phonebookEntries.add(createPhonebookEntry(SECONDUSER_USERNAME, SECONDUSER_FIRSTNAME, USER_LASTNAME));
        m_phonebookEntries.add(createPhonebookEntry("700", "third", "other"));
        m_phonebookEntries.add(createPhonebookEntry("fourthUser", "fourth", USER_LASTNAME));
        m_phonebookEntries.add(createPhonebookEntry(NULLUSER_USERNAME, null, null));
        m_phonebookEntries.add(createPhonebookEntry(LUCENE_STOP_WORD_USERNAME, null, null));
    }

    public Collection<PhonebookEntry> getPhonebookEntries() {
        return Collections.unmodifiableCollection(m_phonebookEntries);
    }

    public Collection<User> getTestUsers() {
        return Collections.unmodifiableCollection(m_usersInPhonebook);
    }

    public PhonebookEntry getEntryByNumber(String number) {
        for (PhonebookEntry entry : m_phonebookEntries) {
            if (entry.getNumber().equals(number)) {
                return entry;
            }
        }
        return null;
    }

    public User getUserByUsername(String username) {
        for (User user : m_usersInPhonebook) {
            if (user.getUserName().equals(username)) {
                return user;
            }
        }
        return null;
    }

    /**
     * Get all numeric extensions for the User and/or PhonebookEntry specified by id
     *
     * @param id
     * @return
     */
    public Collection<String> getExtensions(String id) {
        Collection<String> allExtensions = new ArrayList<String>();
        if (getUserByUsername(id) != null) {
            allExtensions.addAll(getExtensionsForUser(getUserByUsername(id)));
        }
        if (getEntryByNumber(id) != null) {
            String entryExtension = getExtensionForEntry(getEntryByNumber(id));
            if (entryExtension != null) {
                allExtensions.add(entryExtension);
            }
        }

        return allExtensions;
    }

    /**
     * Get all non-numeric sip ids for the User and/or PhonebookEntry specified by id
     *
     * @param id
     * @return
     */
    public Collection<String> getSipIds(String id) {
        Collection<String> allSipIds = new ArrayList<String>();
        if (getUserByUsername(id) != null) {
            allSipIds.addAll(getSipIdsForUser(getUserByUsername(id)));
        }
        if (getEntryByNumber(id) != null) {
            String entryExtension = getExtensionForEntry(getEntryByNumber(id));
            if (entryExtension == null) {
                allSipIds.add(id);
            }
        }

        return allSipIds;
    }

    /**
     * Configures a core context mock object that was created with EasyMock. This method leaves
     * the mock open for further configuration... i.e. replay() is not called
     *
     * @param coreContextMock
     */
    public void configureCoreContextMock(CoreContext coreContextMock) {
        for (PhonebookEntry entry : getPhonebookEntries()) {
            String number = entry.getNumber();
            coreContextMock.loadUserByUserName(number);
            EasyMock.expectLastCall().andReturn(getUserByUsername(number)).anyTimes();
        }
    }

    private Collection<String> getExtensionsForUser(User user) {
        Collection<String> extensions = new ArrayList<String>();
        if (user.getUserName().matches(NUMBER_PATTERN)) {
            extensions.add(user.getUserName());
        }
        for (String alias : user.getAliases()) {
            if (alias.matches(NUMBER_PATTERN)) {
                extensions.add(alias);
            }
        }

        return extensions;
    }

    private Collection<String> getSipIdsForUser(User user) {
        Collection<String> sipIds = new ArrayList<String>();
        if (!user.getUserName().matches(NUMBER_PATTERN)) {
            sipIds.add(user.getUserName());
        }
        for (String alias : user.getAliases()) {
            if (!alias.matches(NUMBER_PATTERN)) {
                sipIds.add(alias);
            }
        }

        return sipIds;
    }

    private String getExtensionForEntry(PhonebookEntry entry) {
        if (entry.getNumber().matches(NUMBER_PATTERN)) {
            return entry.getNumber();
        }

        return null;
    }

    private User createUser(String username, String firstName, String lastName, String aliasString) {
        User user = new User();
        user.setName(username);
        user.setFirstName(firstName);
        user.setLastName(lastName);
        user.setAliasesString(aliasString);
        return user;
    }

    private PhonebookEntry createPhonebookEntry(final String number, final String firstName, final String lastName) {
        PhonebookEntry entry = new PhonebookEntry() {
            public String getNumber() {
                return number;
            }

            public String getLastName() {
                return lastName;
            }

            public String getFirstName() {
                return firstName;
            }

            public AddressBookEntry getAddressBookEntry() {
                return null;
            }
        };

        return entry;
    }
}
