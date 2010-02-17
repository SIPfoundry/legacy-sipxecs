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

import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;

public class GmailPhonebookCsvHeaderTest extends TestCase {

    String[] gmailHeaderStrings = {
        "Name", "Given Name", "Additional Name", "Family Name", "Yomi Name", "Given Name Yomi",
        "Additional Name Yomi", "Family Name Yomi", "Name Prefix", "Name Suffix", "Initials", "Nickname",
        "Short Name", "Maiden Name", "Birthday", "Gender", "Location", "Billing Information", "Directory Server",
        "Mileage", "Occupation", "Hobby", "Sensitivity", "Priority", "Subject", "Notes", "Group Membership",
        "E-mail 1 - Type", "E-mail 1 - Value", "E-mail 2 - Value", "IM 1 - Type", "IM 1 - Service", "IM 1 - Value",
        "Phone 1 - Type", "Phone 1 - Value", "Phone 2 - Type", "Phone 2 - Value", "Phone 3 - Type",
        "Phone 3 - Value", "Address 1 - Type", "Address 1 - Formatted", "Address 1 - Street", "Address 1 - City",
        "Address 1 - PO Box", "Address 1 - Region", "Address 1 - Postal Code", "Address 1 - Country",
        "Address 1 - Extended Address", "Organization 1 - Type", "Organization 1 - Name",
        "Organization 1 - Yomi Name", "Organization 1 - Title", "Organization 1 - Department",
        "Organization 1 - Symbol", "Organization 1 - Location", "Organization 1 - Job Description",
        "Relation 1 - Type", "Relation 1 - Value"
    };
    String[] gmailRow = {
        "Seepgood Guybrush", "Guybrush", "", "Seepgood", "", "", "", "", "", "", "", "", "", "", "1981-02-18", "",
        "", "", "", "", "", "", "", "", "", "", "* My Contacts", "* ", "guybrush@monkey.net",
        "guybrush.seepgood@gmail.com", "Other", "Yahoo!", "3headedmonkey", "Home", "7777", "Mobile", "8888",
        "Work Fax", "8000", "Work", "Melee Island", "", "", "", "", "", "", "", "", "LeChuck Investments", "",
        "pirate wannabe", "", "", "", "", "Assistant", "Murray",
    };

    public void testGetValuesForEntryGmail() {
        Map<String, Integer> header = new HashMap<String, Integer>();
        Integer index = 0;
        for (String headerString : gmailHeaderStrings) {
            header.put(headerString, index++);
        }
        PhonebookFileEntryHelper helper = new GmailPhonebookCsvHeader(header);
        assertEquals("Guybrush", helper.getFirstName(gmailRow));
        assertEquals("Seepgood", helper.getLastName(gmailRow));
        assertEquals("7777", helper.getNumber(gmailRow));
        AddressBookEntry abe = helper.getAddressBookEntry(gmailRow);
        assertEquals("Murray", abe.getAssistantName());
        assertEquals("LeChuck Investments", abe.getCompanyName());
        assertEquals("pirate wannabe", abe.getJobTitle());
        assertEquals("3headedmonkey", abe.getAlternateImId());
        assertEquals("guybrush@monkey.net", abe.getEmailAddress());
        assertEquals("guybrush.seepgood@gmail.com", abe.getAlternateEmailAddress());
    }
}
