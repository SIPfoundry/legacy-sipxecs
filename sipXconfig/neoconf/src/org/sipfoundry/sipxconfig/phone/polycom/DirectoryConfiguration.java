/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.List;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class DirectoryConfiguration extends ProfileContext {
    private final Collection<PhonebookEntry> m_entries;
    private List<Button> m_buttons;

    public DirectoryConfiguration(Collection<PhonebookEntry> entries, SpeedDial speedDial) {
        super(null, "polycom/mac-address-directory.xml.vm");
        m_entries = entries;
        if (speedDial != null) {
            m_buttons = speedDial.getButtons();
        }
    }

    public Collection<PolycomPhonebookEntry> getRows() {
        int size = getSize();
        if (size == 0) {
            return Collections.emptyList();
        }
        Collection<PolycomPhonebookEntry> polycomEntries = new LinkedHashSet<PolycomPhonebookEntry>(size);
        if (m_buttons != null) {
            transformSpeedDial(m_buttons, polycomEntries);
        }
        if (m_entries != null) {
            transformPhoneBook(m_entries, polycomEntries);
        }
        return polycomEntries;
    }

    /**
     * Not all buttons will get into Polycom directory: size might be actually larger than
     */
    private int getSize() {
        int size = 0;
        if (m_entries != null) {
            size += m_entries.size();
        }
        if (m_buttons != null) {
            size += m_buttons.size();
        }
        return size;
    }

    void transformSpeedDial(List<Button> buttons, Collection<PolycomPhonebookEntry> polycomEntries) {
        for (int i = 0; i < buttons.size(); i++) {
            Button button = buttons.get(i);
            if (button.isBlf()) {
                // on Polycom phone BLF entries are handled by RLS server - not by directory
                continue;
            }

            // speed dial entries start with 1 (1..9999)
            polycomEntries.add(new PolycomPhonebookEntry(button, i + 1));
        }
    }

    void transformPhoneBook(Collection<PhonebookEntry> phonebookEntries,
            Collection<PolycomPhonebookEntry> polycomEntries) {
        for (PhonebookEntry entry : phonebookEntries) {
            polycomEntries.add(new PolycomPhonebookEntry(entry));
        }
    }

    /**
     * Due to Polycom limitation all entries with the same contact are equal.
     */
    public static class PolycomPhonebookEntry {
        private final String m_firstName;
        private String m_lastName;
        private final String m_contact;
        private int m_speedDial = -1;

        public PolycomPhonebookEntry(PhonebookEntry entry) {
            m_contact = entry.getNumber();
            m_lastName = entry.getLastName();
            m_firstName = entry.getFirstName();
        }

        public PolycomPhonebookEntry(Button button, int speedDial) {
            m_contact = button.getNumber();
            m_firstName = button.getLabel();
            m_speedDial = speedDial;
        }

        public String getFirstName() {
            // username if first and last name are null. Otherwise it creates a
            // contact entry with no display label which is useless on polycom.
            String firstName = m_firstName;
            if (firstName == null && m_lastName == null) {
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

        public int getSpeedDial() {
            return m_speedDial;
        }

        @Override
        public int hashCode() {
            return new HashCodeBuilder().append(m_contact).toHashCode();
        }

        @Override
        public boolean equals(Object obj) {
            if (!(obj instanceof PolycomPhonebookEntry)) {
                return false;
            }
            if (this == obj) {
                return true;
            }
            PolycomPhonebookEntry rhs = (PolycomPhonebookEntry) obj;
            return new EqualsBuilder().append(m_contact, rhs.m_contact).isEquals();
        }
    }
}
