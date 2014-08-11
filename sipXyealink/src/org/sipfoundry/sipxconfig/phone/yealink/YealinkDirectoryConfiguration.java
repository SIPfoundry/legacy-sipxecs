/*
 * Copyright (c) 2013 SibTelCom, JSC (SIPLABS Communications). All rights reserved.
 * Contributed to SIPfoundry and eZuce, Inc. under a Contributor Agreement.
 *
 * Developed by Konstantin S. Vishnivetsky
 *
 * This library or application is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License (AGPL) as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option) any later version.
 *
 * This library or application is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License (AGPL) for
 * more details.
 *
 */

package org.sipfoundry.sipxconfig.phone.yealink;

import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.List;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;

public class YealinkDirectoryConfiguration extends ProfileContext {
    private final Collection<PhonebookEntry> m_phonebookEntries;

    public YealinkDirectoryConfiguration(YealinkPhone device, Collection<PhonebookEntry> phonebookEntries,
            String profileTemplate) {
        super(device, profileTemplate);
        m_phonebookEntries = phonebookEntries;
    }

    public Collection<YealinkPhonebookEntry> getRows() {
        int size = getSize();
        if (size == 0) {
            return Collections.emptyList();
        }
        Collection<YealinkPhonebookEntry> yealinkEntries = new LinkedHashSet<YealinkPhonebookEntry>(size);
        if (null != m_phonebookEntries) {
            transformPhoneBook(m_phonebookEntries, yealinkEntries);
        }
        return yealinkEntries;
    }

    private int getSize() {
        return null != m_phonebookEntries ? m_phonebookEntries.size() : 0;
    }

    void transformPhoneBook(Collection<PhonebookEntry> phonebookEntries,
            Collection<YealinkPhonebookEntry> yealinkEntries) {
        for (PhonebookEntry entry : phonebookEntries) {
            yealinkEntries.add(new YealinkPhonebookEntry(entry));
        }
        List<YealinkPhonebookEntry> tmp = Collections.list(Collections.enumeration(yealinkEntries));
        Collections.sort(tmp);
        yealinkEntries.clear();
        for (YealinkPhonebookEntry entry : tmp) {
            yealinkEntries.add(entry);
        }
    }

    /**
     * Due to Yealink limitation all entries with the same contact are equal.
     */
    public static class YealinkPhonebookEntry implements Comparable<YealinkPhonebookEntry> {
        private final String m_firstName;
        private String m_lastName;
        private final String m_contact;

        public YealinkPhonebookEntry(PhonebookEntry entry) {
            m_contact = entry.getNumber();
            m_lastName = entry.getLastName();
            m_firstName = entry.getFirstName();
        }

        public String getFirstName() {
            String firstName = m_firstName;
            if (null == firstName && null == m_lastName) {
                return m_contact;
            }
            return firstName;
        }

        public String getLastName() {
            return m_lastName;
        }

        public String getContact() {
            return m_contact;
        }

        @Override
        public int hashCode() {
            return new HashCodeBuilder().append(m_contact).toHashCode();
        }

        @Override
        public int compareTo(YealinkPhonebookEntry a) {
            int result = 0;
            if (null == a) {
                return 0;
            }
            if (null != m_lastName && null != a.getLastName()) {
                result = m_lastName.compareTo(a.getLastName());
            }
            if (null != m_firstName && null != a.getFirstName()) {
                return result == 0 ? m_firstName.compareTo(a.getFirstName()) : result;
            } else {
                return result;
            }
        }

        @Override
        public boolean equals(Object obj) {
            if (!(obj instanceof YealinkPhonebookEntry)) {
                return false;
            }
            if (this == obj) {
                return true;
            }
            YealinkPhonebookEntry rhs = (YealinkPhonebookEntry) obj;
            return new EqualsBuilder().append(m_contact, rhs.m_contact).isEquals();
        }
    }
}
