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

public interface PhonebookFileEntryHelper {
    String getFirstName(String[] values);

    String getLastName(String[] values);

    String getNumber(String[] values);

    AddressBookEntry getAddressBookEntry(String[] values);
}
