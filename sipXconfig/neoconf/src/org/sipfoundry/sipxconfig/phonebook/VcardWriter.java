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

        if (StringUtils.isEmpty(entry.getFirstName()) && StringUtils.isEmpty(entry.getLastName())) {
            return;
        }

        String firstName = StringUtils.defaultString(entry.getFirstName());
        String lastName = StringUtils.defaultString(entry.getLastName());
        String phoneNumber = StringUtils.defaultString(entry.getNumber());

        String cellPhoneNumber = null;
        String homePhoneNumber = null;
        String faxNumber = null;

        String emailAddress = null;
        String alternateEmailAddress = null;

        String companyName = null;
        String jobTitle = null;
        String jobDept = null;

        String homeAddressStreet = null;
        String homeAddressZip = null;
        String homeAddressCountry = null;
        String homeAddressState = null;
        String homeAddressCity = null;

        String officeAddressStreet = null;
        String officeAddressZip = null;
        String officeAddressCountry = null;
        String officeAddressState = null;
        String officeAddressCity = null;
        String officeAddressOfficeDesignation = null;

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

            homeAddressStreet = StringUtils.defaultString(addressBook.getHomeAddress().getStreet());
            homeAddressZip = StringUtils.defaultString(addressBook.getHomeAddress().getZip());
            homeAddressCountry = StringUtils.defaultString(addressBook.getHomeAddress().getCountry());
            homeAddressState = StringUtils.defaultString(addressBook.getHomeAddress().getState());
            homeAddressCity = StringUtils.defaultString(addressBook.getHomeAddress().getCity());

            officeAddressStreet = StringUtils.defaultString(addressBook.getOfficeAddress().getStreet());
            officeAddressZip = StringUtils.defaultString(addressBook.getOfficeAddress().getZip());
            officeAddressCountry = StringUtils.defaultString(addressBook.getOfficeAddress().getCountry());
            officeAddressState = StringUtils.defaultString(addressBook.getOfficeAddress().getState());
            officeAddressCity = StringUtils.defaultString(addressBook.getOfficeAddress().getCity());
            officeAddressOfficeDesignation = StringUtils.defaultString(addressBook.getOfficeAddress()
                    .getOfficeDesignation());
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
