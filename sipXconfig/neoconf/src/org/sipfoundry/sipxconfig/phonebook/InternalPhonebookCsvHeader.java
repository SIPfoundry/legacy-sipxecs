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

public class InternalPhonebookCsvHeader implements PhonebookFileEntryHelper {
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
        if (row.length < 27) {
            return null;
        }

        AddressBookEntry abe = new AddressBookEntry();
        abe.setJobTitle(row[3]);
        abe.setJobDept(row[4]);
        abe.setCompanyName(row[5]);
        abe.setAssistantName(row[6]);
        abe.setCellPhoneNumber(row[7]);
        abe.setHomePhoneNumber(row[8]);
        abe.setAssistantPhoneNumber(row[9]);
        abe.setFaxNumber(row[10]);
        abe.setImId(row[11]);
        abe.setAlternateImId(row[12]);
        abe.setLocation(row[13]);

        Address homeAddress = new Address();
        homeAddress.setCity(row[14]);
        homeAddress.setCountry(row[15]);
        homeAddress.setState(row[16]);
        homeAddress.setStreet(row[17]);
        homeAddress.setZip(row[18]);
        abe.setHomeAddress(homeAddress);

        Address officeAddress = new Address();
        officeAddress.setCity(row[19]);
        officeAddress.setCountry(row[20]);
        officeAddress.setState(row[21]);
        officeAddress.setStreet(row[22]);
        officeAddress.setZip(row[23]);
        officeAddress.setOfficeDesignation(row[24]);
        abe.setOfficeAddress(officeAddress);
        abe.setEmailAddress(row[25]);
        abe.setAlternateEmailAddress(row[26]);

        return abe;
    }
}
