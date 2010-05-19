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

import java.io.StringWriter;
import org.apache.commons.io.IOUtils;
import junit.framework.TestCase;

public class VcardWriterTest extends TestCase {

    public void testWrite() throws Exception {
        StringWriter writer = new StringWriter();
        VcardWriter cardWriter = new VcardWriter(writer);

        PhonebookEntry entry = new PhonebookEntry();
        entry.setFirstName("Michael");
        entry.setLastName("Jordan");
        entry.setNumber("1234");
        AddressBookEntry abe = new AddressBookEntry();
        abe.setHomePhoneNumber("2222");
        abe.setCellPhoneNumber("333");
        abe.setFaxNumber("555");
        Address workAddress = new Address();
        workAddress.setOfficeDesignation("OfficeDesignation");
        workAddress.setStreet("OfficeStreet");
        workAddress.setCity("OfficeCity");
        workAddress.setState("OfficeState");
        workAddress.setCountry("OfficeCountry");
        abe.setOfficeAddress(workAddress);
        Address homeAddress = new Address();
        homeAddress.setStreet("HomeStreet");
        homeAddress.setCity("HomeCity");
        homeAddress.setState("HomeState");
        homeAddress.setCountry("HomeCountry");
        abe.setHomeAddress(homeAddress);

        abe.setEmailAddress("test@test.com");
        abe.setAlternateEmailAddress("alt@test.com");
        abe.setCompanyName("NBA");
        abe.setJobTitle("Mr.");

        entry.setAddressBookEntry(abe);
        cardWriter.write(entry);

        cardWriter.write(null);

        PhonebookEntry noEntry = new PhonebookEntry();
        cardWriter.write(noEntry);

        PhonebookEntry entry1 = new PhonebookEntry();
        entry1.setFirstName("Teemu");
        entry1.setLastName("Selanne");
        entry1.setNumber("789");
        cardWriter.write(entry1);

        PhonebookEntry entry2 = new PhonebookEntry();
        entry2.setFirstName("Saku");
        entry2.setLastName("Koivu");
        entry2.setNumber("5432");
        AddressBookEntry abe2 = new AddressBookEntry();
        entry2.setAddressBookEntry(abe2);
        cardWriter.write(entry2);

        PhonebookEntry entry3 = new PhonebookEntry();
        entry3.setFirstName("Eric");
        entry3.setLastName("Staal");
        entry3.setNumber("4343");
        AddressBookEntry abe3 = new AddressBookEntry();
        Address office = new Address();
        abe3.setOfficeAddress(office);
        Address home = new Address();
        abe3.setHomeAddress(home);
        entry3.setAddressBookEntry(abe3);
        cardWriter.write(entry3);

        assertEquals(IOUtils.toString(getClass().getResourceAsStream("phonebook_test.vcf")), writer
                .getBuffer().toString());
    }
}
