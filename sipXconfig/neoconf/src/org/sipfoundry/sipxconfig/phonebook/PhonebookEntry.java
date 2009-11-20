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

import org.sipfoundry.sipxconfig.common.BeanWithId;

public class PhonebookEntry extends BeanWithId {
    private String m_firstName;
    private String m_lastName;
    private String m_number;
    private AddressBookEntry m_addressBookEntry;
    private Phonebook m_phonebook;

    public String getFirstName() {
        return m_firstName;
    }

    public void setFirstName(String firstName) {
        m_firstName = firstName;
    }

    public String getLastName() {
        return m_lastName;
    }

    public void setLastName(String lastName) {
        m_lastName = lastName;
    }

    public String getNumber() {
        return m_number;
    }

    public void setNumber(String number) {
        m_number = number;
    }

    public AddressBookEntry getAddressBookEntry() {
        return m_addressBookEntry;
    }

    public void setAddressBookEntry(AddressBookEntry addressBookEntry) {
        m_addressBookEntry = addressBookEntry;
    }

    public Phonebook getPhonebook() {
        return m_phonebook;
    }

    public void setPhonebook(Phonebook phonebook) {
        m_phonebook = phonebook;
    }
}
