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

public class GooglePhonebookCsvHeaderTest extends TestCase {

    String[] gmailHeaderStrings = {
        "Name", "Given Name", "Additional Name", "Family Name", "Yomi Name", "Given Name Yomi",
        "Additional Name Yomi", "Family Name Yomi", "Name Prefix", "Name Suffix", "Initials", "Nickname",
        "Short Name", "Maiden Name", "Birthday", "Gender", "Location", "Billing Information", "Directory Server",
        "Mileage", "Occupation", "Hobby", "Sensitivity", "Priority", "Subject", "Notes", "Group Membership",
        "IM 1 - Type", "IM 1 - Service", "IM 1 - Value", "Phone 1 - Type", "Phone 1 - Value", "Phone 2 - Type",
        "Phone 2 - Value", "Phone 3 - Type", "Phone 3 - Value", "Address 1 - Type", "Address 1 - Formatted",
        "Address 1 - Street", "Address 1 - City", "Address 1 - PO Box", "Address 1 - Region",
        "Address 1 - Postal Code", "Address 1 - Country", "Address 1 - Extended Address", "Organization 1 - Type",
        "Organization 1 - Name", "Organization 1 - Yomi Name", "Organization 1 - Title",
        "Organization 1 - Department", "Organization 1 - Symbol", "Organization 1 - Location",
        "Organization 1 - Job Description", "Relation 1 - Type", "Relation 1 - Value", "E-mail 1 - Type",
        "E-mail 1 - Value", "E-mail 2 - Type", "E-mail 2 - Value", "E-mail 3 - Type", "E-mail 3 - Value",
        "E-mail 4 - Type", "E-mail 4 - Value"
    };
    String[] gmailRow = {
        "Seepgood Guybrush", "Guybrush", "", "Seepgood", "", "", "", "", "", "", "", "", "", "", "1981-02-18", "",
        "", "", "", "", "", "", "", "", "", "", "* My Contacts", "Other", "Yahoo!", "3headedmonkey", "Home", "7777",
        "Mobile", "8888", "Work Fax", "8000", "Work", "Melee Island", "", "", "", "", "", "", "", "",
        "LeChuck Investments", "", "pirate wannabe", "", "", "", "", "Assistant", "Murray", "* Work",
        "guybrush@monkey.net", "Home", "guybrush.seepgood@gmail.com", "Home", "tralala@yahoo.com", "Home",
        "oohlala@gmail.com"
    };

    public void testGetValuesForEntryGoogle() {
        Map<String, Integer> header = new HashMap<String, Integer>();
        Integer index = 0;
        for (String headerString : gmailHeaderStrings) {
            header.put(headerString, index++);
        }
        PhonebookFileEntryHelper helper = new GooglePhonebookCsvHeader(header);
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

    public void testGetEmails() {
        String[] row1 = {
            "Seepgood Guybrush", "Guybrush", "", "Seepgood", "", "", "", "", "", "", "", "", "", "", "1981-02-18",
            "", "", "", "", "", "", "", "", "", "", "", "* My Contacts", "Other", "Yahoo!", "3headedmonkey", "Home",
            "7777", "Mobile", "8888", "Work Fax", "8000", "Work", "Melee Island", "", "", "", "", "", "", "", "",
            "LeChuck Investments", "", "pirate wannabe", "", "", "", "", "Assistant", "Murray", "* Home",
            "jean.luc@gmail.com", "Home", "guybrush@monkey.net", "Home", "guybrush.seepgood@gmail.com", "Work",
            "cpt.picard@starfleet.mw"
        };
        String[] row2 = {
            "Seepgood Guybrush", "Guybrush", "", "Seepgood", "", "", "", "", "", "", "", "", "", "", "1981-02-18",
            "", "", "", "", "", "", "", "", "", "", "", "* My Contacts", "Other", "Yahoo!", "3headedmonkey", "Home",
            "7777", "Mobile", "8888", "Work Fax", "8000", "Work", "Melee Island", "", "", "", "", "", "", "", "",
            "LeChuck Investments", "", "pirate wannabe", "", "", "", "", "Assistant", "Murray", "* Work",
            "cpt.picard@starfleet.mw", "* Home", "jean.luc@gmail.com", "Home", "guybrush@monkey.net", "Home",
            "guybrush.seepgood@gmail.com"
        };
        String[] row3 = {
            "Seepgood Guybrush", "Guybrush", "", "Seepgood", "", "", "", "", "", "", "", "", "", "", "1981-02-18",
            "", "", "", "", "", "", "", "", "", "", "", "* My Contacts", "Other", "Yahoo!", "3headedmonkey", "Home",
            "7777", "Mobile", "8888", "Work Fax", "8000", "Work", "Melee Island", "", "", "", "", "", "", "", "",
            "LeChuck Investments", "", "pirate wannabe", "", "", "", "", "Assistant", "Murray", "* Work",
            "cpt.picard@starfleet.mw", "Work", "jean.luc@gmail.com", "Work", "guybrush@monkey.net", "Work",
            "guybrush.seepgood@gmail.com"
        };
        String[] gmailHeaderNoEmails = {
            "Name", "Given Name", "Additional Name", "Family Name", "Yomi Name", "Given Name Yomi",
            "Additional Name Yomi", "Family Name Yomi", "Name Prefix", "Name Suffix", "Initials", "Nickname",
            "Short Name", "Maiden Name", "Birthday", "Gender", "Location", "Billing Information",
            "Directory Server", "Mileage", "Occupation", "Hobby", "Sensitivity", "Priority", "Subject", "Notes",
            "Group Membership", "IM 1 - Type", "IM 1 - Service", "IM 1 - Value", "Phone 1 - Type",
            "Phone 1 - Value", "Phone 2 - Type", "Phone 2 - Value", "Phone 3 - Type", "Phone 3 - Value",
            "Address 1 - Type", "Address 1 - Formatted", "Address 1 - Street", "Address 1 - City",
            "Address 1 - PO Box", "Address 1 - Region", "Address 1 - Postal Code", "Address 1 - Country",
            "Address 1 - Extended Address", "Organization 1 - Type", "Organization 1 - Name",
            "Organization 1 - Yomi Name", "Organization 1 - Title", "Organization 1 - Department",
            "Organization 1 - Symbol", "Organization 1 - Location", "Organization 1 - Job Description",
            "Relation 1 - Type", "Relation 1 - Value"
        };
        String[] row4NoEmails = {
            "Seepgood Guybrush", "Guybrush", "", "Seepgood", "", "", "", "", "", "", "", "", "", "", "1981-02-18",
            "", "", "", "", "", "", "", "", "", "", "", "* My Contacts", "Other", "Yahoo!", "3headedmonkey", "Home",
            "7777", "Mobile", "8888", "Work Fax", "8000", "Work", "Melee Island", "", "", "", "", "", "", "", "",
            "LeChuck Investments", "", "pirate wannabe", "", "", "", "", "Assistant", "Murray"
        };
        String[] gmailHeaderOneEmailWork = {
            "Name", "Given Name", "Additional Name", "Family Name", "Yomi Name", "Given Name Yomi",
            "Additional Name Yomi", "Family Name Yomi", "Name Prefix", "Name Suffix", "Initials", "Nickname",
            "Short Name", "Maiden Name", "Birthday", "Gender", "Location", "Billing Information",
            "Directory Server", "Mileage", "Occupation", "Hobby", "Sensitivity", "Priority", "Subject", "Notes",
            "Group Membership", "IM 1 - Type", "IM 1 - Service", "IM 1 - Value", "Phone 1 - Type",
            "Phone 1 - Value", "Phone 2 - Type", "Phone 2 - Value", "Phone 3 - Type", "Phone 3 - Value",
            "Address 1 - Type", "Address 1 - Formatted", "Address 1 - Street", "Address 1 - City",
            "Address 1 - PO Box", "Address 1 - Region", "Address 1 - Postal Code", "Address 1 - Country",
            "Address 1 - Extended Address", "Organization 1 - Type", "Organization 1 - Name",
            "Organization 1 - Yomi Name", "Organization 1 - Title", "Organization 1 - Department",
            "Organization 1 - Symbol", "Organization 1 - Location", "Organization 1 - Job Description",
            "Relation 1 - Type", "Relation 1 - Value", "E-mail 1 - Type", "E-mail 1 - Value"
        };
        String[] row5OneEmailWork = {
            "Seepgood Guybrush", "Guybrush", "", "Seepgood", "", "", "", "", "", "", "", "", "", "", "1981-02-18",
            "", "", "", "", "", "", "", "", "", "", "", "* My Contacts", "Other", "Yahoo!", "3headedmonkey", "Home",
            "7777", "Mobile", "8888", "Work Fax", "8000", "Work", "Melee Island", "", "", "", "", "", "", "", "",
            "LeChuck Investments", "", "pirate wannabe", "", "", "", "", "Assistant", "Murray", "* Work",
            "cpt.picard@starfleet.mw"
        };
        String[] gmailHeaderOneEmailHome = {
            "Name", "Given Name", "Additional Name", "Family Name", "Yomi Name", "Given Name Yomi",
            "Additional Name Yomi", "Family Name Yomi", "Name Prefix", "Name Suffix", "Initials", "Nickname",
            "Short Name", "Maiden Name", "Birthday", "Gender", "Location", "Billing Information",
            "Directory Server", "Mileage", "Occupation", "Hobby", "Sensitivity", "Priority", "Subject", "Notes",
            "Group Membership", "IM 1 - Type", "IM 1 - Service", "IM 1 - Value", "Phone 1 - Type",
            "Phone 1 - Value", "Phone 2 - Type", "Phone 2 - Value", "Phone 3 - Type", "Phone 3 - Value",
            "Address 1 - Type", "Address 1 - Formatted", "Address 1 - Street", "Address 1 - City",
            "Address 1 - PO Box", "Address 1 - Region", "Address 1 - Postal Code", "Address 1 - Country",
            "Address 1 - Extended Address", "Organization 1 - Type", "Organization 1 - Name",
            "Organization 1 - Yomi Name", "Organization 1 - Title", "Organization 1 - Department",
            "Organization 1 - Symbol", "Organization 1 - Location", "Organization 1 - Job Description",
            "Relation 1 - Type", "Relation 1 - Value", "E-mail 1 - Type", "E-mail 1 - Value"
        };
        String[] row6OneEmailHome = {
            "Seepgood Guybrush", "Guybrush", "", "Seepgood", "", "", "", "", "", "", "", "", "", "", "1981-02-18",
            "", "", "", "", "", "", "", "", "", "", "", "* My Contacts", "Other", "Yahoo!", "3headedmonkey", "Home",
            "7777", "Mobile", "8888", "Work Fax", "8000", "Work", "Melee Island", "", "", "", "", "", "", "", "",
            "LeChuck Investments", "", "pirate wannabe", "", "", "", "", "Assistant", "Murray", "* Other",
            "cpt.picard@starfleet.mw"
        };
        Map<String, Integer> header = new HashMap<String, Integer>();
        Integer index = 0;
        for (String headerString : gmailHeaderStrings) {
            header.put(headerString, index++);
        }
        PhonebookFileEntryHelper helper = new GooglePhonebookCsvHeader(header);
        AddressBookEntry abe1 = helper.getAddressBookEntry(row1);
        assertEquals("cpt.picard@starfleet.mw", abe1.getEmailAddress());
        assertEquals("jean.luc@gmail.com", abe1.getAlternateEmailAddress());

        AddressBookEntry abe2 = helper.getAddressBookEntry(row2);
        assertEquals("cpt.picard@starfleet.mw", abe2.getEmailAddress());
        assertEquals("jean.luc@gmail.com", abe2.getAlternateEmailAddress());

        AddressBookEntry abe3 = helper.getAddressBookEntry(row3);
        assertEquals("cpt.picard@starfleet.mw", abe3.getEmailAddress());
        assertNull(abe3.getAlternateEmailAddress());

        Map<String, Integer> headerNoEmails = new HashMap<String, Integer>();
        Integer indexNoEmails = 0;
        for (String headerString : gmailHeaderNoEmails) {
            headerNoEmails.put(headerString, indexNoEmails++);
        }
        PhonebookFileEntryHelper helperNoEmails = new GooglePhonebookCsvHeader(headerNoEmails);
        AddressBookEntry abe4 = helperNoEmails.getAddressBookEntry(row4NoEmails);
        assertNull(abe4.getEmailAddress());
        assertNull(abe4.getAlternateEmailAddress());

        Map<String, Integer> headerOneEmailWork = new HashMap<String, Integer>();
        Integer indexOneEmailWork = 0;
        for (String headerString : gmailHeaderOneEmailWork) {
            headerOneEmailWork.put(headerString, indexOneEmailWork++);
        }
        PhonebookFileEntryHelper helperOneEmailWork = new GooglePhonebookCsvHeader(headerOneEmailWork);
        AddressBookEntry abe5 = helperOneEmailWork.getAddressBookEntry(row5OneEmailWork);
        assertEquals("cpt.picard@starfleet.mw", abe5.getEmailAddress());
        assertNull(abe5.getAlternateEmailAddress());

        Map<String, Integer> headerOneEmailHome = new HashMap<String, Integer>();
        Integer indexOneEmailHome = 0;
        for (String headerString : gmailHeaderOneEmailHome) {
            headerOneEmailHome.put(headerString, indexOneEmailHome++);
        }
        PhonebookFileEntryHelper helperOneEmailHome = new GooglePhonebookCsvHeader(headerOneEmailHome);
        AddressBookEntry abe6 = helperOneEmailHome.getAddressBookEntry(row6OneEmailHome);
        assertEquals("cpt.picard@starfleet.mw", abe6.getAlternateEmailAddress());
        assertNull(abe6.getEmailAddress());
    }
}
