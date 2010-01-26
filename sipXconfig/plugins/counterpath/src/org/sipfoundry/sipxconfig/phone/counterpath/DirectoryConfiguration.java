/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.counterpath;

import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashSet;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phonebook.Address;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;

public class DirectoryConfiguration extends ProfileContext {
    private static final String EMPTY_STRING = "";
    private static final String SINGLE_SPACE = " ";
    private final Collection<PhonebookEntry> m_entries;
    private String m_domainName;

    public DirectoryConfiguration(Collection<PhonebookEntry> entries, String domainName) {
        super(null, "counterpath/contacts-resource-list.xml.vm");
        m_entries = entries;
        m_domainName = domainName;
    }

    public Collection<CounterpathPhonebookEntry> getRows() {
        int size = getSize();
        if (size == 0) {
            return Collections.emptyList();
        }
        Collection<CounterpathPhonebookEntry> counterpathEntries = new LinkedHashSet<CounterpathPhonebookEntry>(size);
        if (m_entries != null) {
            transformPhoneBook(m_entries, counterpathEntries, m_domainName);
        }
        return counterpathEntries;
    }

    /**
     * Not all buttons will get into Polycom directory: size might be actually larger than
     */
    private int getSize() {
        int size = 0;
        if (m_entries != null) {
            size += m_entries.size();
        }
        return size;
    }

    void transformPhoneBook(Collection<PhonebookEntry> phonebookEntries,
            Collection<CounterpathPhonebookEntry> counterpathEntries, String domainName) {
        for (PhonebookEntry entry : phonebookEntries) {
            counterpathEntries.add(new CounterpathPhonebookEntry(entry, domainName));
        }
    }

    /**
     * Due to Polycom limitation all entries with the same contact are equal.
     */
    public static class CounterpathPhonebookEntry {

        private PhonebookEntry m_entry;
        private AddressBookEntry m_subEntry;
        private String m_domainName;

        public CounterpathPhonebookEntry(PhonebookEntry entry, String domainName) {
            m_entry = entry;
            m_subEntry = entry.getAddressBookEntry();
            m_domainName = domainName;
        }

        public String getUri() {
            return "sip:" + m_entry.getNumber() + "@" + m_domainName;
        }

        public String getGivenName() {
            return m_entry.getFirstName() == null ? EMPTY_STRING : m_entry.getFirstName();
        }

        public String getLastName() {
            return m_entry.getLastName() == null ? EMPTY_STRING : m_entry.getLastName();
        }

        public String getBusinessNumber() {
            return m_entry.getNumber();
        }

        public String getXmppAddress() {
            if (m_subEntry != null) {
                return m_subEntry.getImId() == null ? EMPTY_STRING : m_subEntry.getImId();
            }

            return EMPTY_STRING;
        }

        public String getHomeNumber() {
            if (m_subEntry != null) {
                return m_subEntry.getHomePhoneNumber() == null ? EMPTY_STRING : m_subEntry.getHomePhoneNumber();
            }
            return EMPTY_STRING;
        }

        public String getMobileNumber() {
            if (m_subEntry != null) {
                return m_subEntry.getCellPhoneNumber() == null ? EMPTY_STRING : m_subEntry.getCellPhoneNumber();
            }
            return EMPTY_STRING;
        }

        public String getFaxNumber() {
            if (m_subEntry != null) {
                return m_subEntry.getFaxNumber() == null ? EMPTY_STRING : m_subEntry.getFaxNumber();
            }
            return EMPTY_STRING;
        }

        public String getHomeAddress() {
            if (m_subEntry != null) {
                return convertToString(m_subEntry.getHomeAddress());
            }
            return EMPTY_STRING;
        }

        public String getOfficeAddress() {
            if (m_subEntry != null) {
                return convertToString(m_subEntry.getOfficeAddress());
            }
            return EMPTY_STRING;
        }

        public String getCompany() {
            if (m_subEntry != null) {
                return m_subEntry.getCompanyName() == null ? EMPTY_STRING : m_subEntry.getCompanyName();
            }
            return EMPTY_STRING;
        }

        public String getJobTitle() {
            if (m_subEntry != null) {
                return m_subEntry.getJobTitle() == null ? EMPTY_STRING : m_subEntry.getJobTitle();
            }
            return EMPTY_STRING;
        }

        private String convertToString(Address address) {

            if (address == null) {
                return EMPTY_STRING;
            }

            StringBuilder addrBuilder = new StringBuilder();

            addrBuilder.append(address.getStreet() == null ? EMPTY_STRING : (address.getStreet() + ","));
            addrBuilder.append(address.getCity() == null ? EMPTY_STRING : (address.getCity() + SINGLE_SPACE));
            addrBuilder.append(address.getState() == null ? EMPTY_STRING : (address.getState() + SINGLE_SPACE));
            addrBuilder.append(address.getCountry() == null ? EMPTY_STRING : (address.getCountry() + SINGLE_SPACE));
            addrBuilder.append(address.getZip() == null ? EMPTY_STRING : address.getZip());

            return addrBuilder.toString();
        }

        @Override
        public int hashCode() {
            return new HashCodeBuilder().append(getUri()).toHashCode();
        }

        @Override
        public boolean equals(Object obj) {
            if (!(obj instanceof CounterpathPhonebookEntry)) {
                return false;
            }
            if (this == obj) {
                return true;
            }
            CounterpathPhonebookEntry rhs = (CounterpathPhonebookEntry) obj;
            return new EqualsBuilder().append(getUri(), rhs.getUri()).isEquals();
        }
    }
}
