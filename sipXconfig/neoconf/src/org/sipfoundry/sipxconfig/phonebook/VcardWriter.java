/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.phonebook;

import java.io.IOException;
import java.io.Writer;
import java.util.Formatter;

import org.apache.commons.lang.StringUtils;

public class VcardWriter {
    public void write(Writer writer, PhonebookEntry entry) throws IOException {

        if (entry == null) {
            return;
        }

        if (StringUtils.isEmpty(entry.getFirstName()) && StringUtils.isEmpty(entry.getLastName())) {
            return;
        }

        String firstName = StringUtils.defaultString(entry.getFirstName());
        String lastName = StringUtils.defaultString(entry.getLastName());
        String phoneNumber = StringUtils.defaultString(entry.getNumber());

        String cellPhoneNumber = StringUtils.EMPTY;
        String homePhoneNumber = StringUtils.EMPTY;
        String faxNumber = StringUtils.EMPTY;

        String emailAddress = StringUtils.EMPTY;
        String alternateEmailAddress = StringUtils.EMPTY;

        String companyName = StringUtils.EMPTY;
        String jobTitle = StringUtils.EMPTY;
        String jobDept = StringUtils.EMPTY;

        String homeAddressStreet = StringUtils.EMPTY;
        String homeAddressZip = StringUtils.EMPTY;
        String homeAddressCountry = StringUtils.EMPTY;
        String homeAddressState = StringUtils.EMPTY;
        String homeAddressCity = StringUtils.EMPTY;

        String officeAddressStreet = StringUtils.EMPTY;
        String officeAddressZip = StringUtils.EMPTY;
        String officeAddressCountry = StringUtils.EMPTY;
        String officeAddressState = StringUtils.EMPTY;
        String officeAddressCity = StringUtils.EMPTY;
        String officeAddressOfficeDesignation = StringUtils.EMPTY;

        AddressBookEntry addressBook = entry.getAddressBookEntry();
        if (addressBook != null) {
            cellPhoneNumber = StringUtils.defaultString(addressBook.getCellPhoneNumber());
            homePhoneNumber = StringUtils.defaultString(addressBook.getHomePhoneNumber());
            faxNumber = StringUtils.defaultString(addressBook.getFaxNumber());

            emailAddress = StringUtils.defaultString(addressBook.getEmailAddress());
            alternateEmailAddress = StringUtils.defaultString(addressBook.getAlternateEmailAddress());

            companyName = StringUtils.defaultString(addressBook.getCompanyName());
            jobTitle = StringUtils.defaultString(addressBook.getJobTitle());
            jobDept = StringUtils.defaultString(addressBook.getJobDept());

            if (addressBook.getHomeAddress() != null) {
                homeAddressStreet = StringUtils.defaultString(addressBook.getHomeAddress().getStreet());
                homeAddressZip = StringUtils.defaultString(addressBook.getHomeAddress().getZip());
                homeAddressCountry = StringUtils.defaultString(addressBook.getHomeAddress().getCountry());
                homeAddressState = StringUtils.defaultString(addressBook.getHomeAddress().getState());
                homeAddressCity = StringUtils.defaultString(addressBook.getHomeAddress().getCity());
            }

            if (addressBook.getOfficeAddress() != null) {
                officeAddressStreet = StringUtils.defaultString(addressBook.getOfficeAddress().getStreet());
                officeAddressZip = StringUtils.defaultString(addressBook.getOfficeAddress().getZip());
                officeAddressCountry = StringUtils.defaultString(addressBook.getOfficeAddress().getCountry());
                officeAddressState = StringUtils.defaultString(addressBook.getOfficeAddress().getState());
                officeAddressCity = StringUtils.defaultString(addressBook.getOfficeAddress().getCity());
                officeAddressOfficeDesignation = StringUtils.defaultString(addressBook.getOfficeAddress()
                        .getOfficeDesignation());
            }
        }

        Formatter formatter = new Formatter(writer);
        writer.write("BEGIN:vCard\n");
        writer.write("VERSION:3.0\n");
        formatter.format("FN:%s %s\n", firstName, lastName);
        formatter.format("N:%s;%s;;;\n", lastName, firstName);
        formatter.format("TEL;TYPE=WORK:%s\n", phoneNumber);
        if (addressBook != null) {
            formatter.format("TEL;TYPE=HOME:%s\n", homePhoneNumber);
            formatter.format("TEL;TYPE=CELL:%s\n", cellPhoneNumber);
            formatter.format("TEL;TYPE=FAX:%s\n", faxNumber);
            formatter.format("ADR;TYPE=WORK:%s;;%s;%s;%s;%s;%s\n", officeAddressOfficeDesignation,
                    officeAddressStreet, officeAddressCity, officeAddressState, officeAddressZip,
                    officeAddressCountry);
            formatter.format("ADR;TYPE=HOME:;;%s;%s;%s;%s;%s\n", homeAddressStreet, homeAddressCity,
                    homeAddressState, homeAddressZip, homeAddressCountry);
            formatter.format("EMAIL;TYPE=PREF:%s\n", emailAddress);
            formatter.format("EMAIL:%s\n", alternateEmailAddress);
            formatter.format("ORG:%s;%s\n", companyName, jobDept);
            formatter.format("TITLE:%s\n", jobTitle);
        }
        writer.write("END:vCard\n\n");
    }
}
