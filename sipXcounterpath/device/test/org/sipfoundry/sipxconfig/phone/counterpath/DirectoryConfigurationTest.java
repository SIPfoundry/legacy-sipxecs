/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.counterpath;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;

import org.custommonkey.xmlunit.Diff;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phonebook.Address;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class DirectoryConfigurationTest extends XMLTestCase {
    private ProfileGenerator m_pg;
    private MemoryProfileLocation m_location;

    @Override
    protected void setUp() {
        XMLUnit.setIgnoreWhitespace(true);

        m_location = new MemoryProfileLocation();
        VelocityProfileGenerator pg = new VelocityProfileGenerator();
        pg.setVelocityEngine(TestHelper.getVelocityEngine());
        m_pg = pg;
    }

    public void testGenerateDirectory() throws Exception {

        PhonebookEntry phonebookEntry = new PhonebookEntry();
        phonebookEntry.setFirstName("Marks & Spencer");
        phonebookEntry.setLastName("<Shop>");
        phonebookEntry.setNumber("'210'");

        AddressBookEntry abe = new AddressBookEntry();
        abe.setImId("'IMId'");
        abe.setHomePhoneNumber("\"1234\"");
        abe.setCellPhoneNumber("'222'");
        abe.setFaxNumber("<1234>");
        Address address = new Address();
        address.setCity("Bucharest&");
        address.setCountry("Country&");
        address.setStreet("<'Street'>");
        address.setZip("\"Zip\"");
        address.setState("\"State\"");
        abe.setHomeAddress(address);
        abe.setOfficeAddress(address);
        abe.setCompanyName("Marks&Spencer");
        abe.setJobTitle("tester&dev");
        phonebookEntry.setAddressBookEntry(abe);

        Collection<PhonebookEntry> entries = Collections.singleton(phonebookEntry);
        DirectoryConfiguration dir = new DirectoryConfiguration(entries, "test.org");

        m_pg.generate(m_location, dir, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-directory.xml");
        Reader expectedXml = new InputStreamReader(expectedPhoneStream);
        Reader generatedXml = m_location.getReader();

        Diff phoneDiff = new Diff(expectedXml, generatedXml);
        assertXMLEqual(phoneDiff, true);
        expectedPhoneStream.close();

    }
}
