/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.phonebook;

public class InternalPhonebookVcardHeader implements PhonebookFileEntryHelper {
    public String getFirstName(String[] row) {
        return row[0];
    }

    public String getLastName(String[] row) {
        return row[1];
    }

    public String getNumber(String[] row) {
        return row[2];
    }

    public AddressBookEntry getAddressBookEntry(String[] row) {
        if (row.length < 22) {
            return null;
        }

        AddressBookEntry abe = new AddressBookEntry();
        abe.setCellPhoneNumber(row[3]);
        abe.setHomePhoneNumber(row[4]);
        abe.setFaxNumber(row[5]);
        abe.setEmailAddress(row[6]);
        abe.setAlternateEmailAddress(row[7]);
        abe.setCompanyName(row[8]);
        abe.setJobTitle(row[9]);
        abe.setJobDept(row[10]);

        Address homeAddress = new Address();
        homeAddress.setStreet(row[11]);
        homeAddress.setZip(row[12]);
        homeAddress.setCountry(row[13]);
        homeAddress.setState(row[14]);
        homeAddress.setCity(row[15]);
        abe.setHomeAddress(homeAddress);

        Address officeAddress = new Address();
        officeAddress.setStreet(row[16]);
        officeAddress.setZip(row[17]);
        officeAddress.setCity(row[18]);
        officeAddress.setOfficeDesignation(row[19]);
        officeAddress.setCountry(row[20]);
        officeAddress.setState(row[21]);
        abe.setOfficeAddress(officeAddress);

        return abe;
    }
}
