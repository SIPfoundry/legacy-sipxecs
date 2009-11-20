/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;

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
import org.sipfoundry.sipxconfig.phone.polycom.DirectoryConfiguration.PolycomPhonebookEntry;
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

    public void testTransformRows() throws Exception {
        PhonebookEntry phonebookEntry = new PhonebookEntry();
        phonebookEntry.setFirstName(null);
        phonebookEntry.setLastName(null);
        phonebookEntry.setNumber("1234");

        DirectoryConfiguration dir = new DirectoryConfiguration(null, null);
        Collection<PolycomPhonebookEntry> collection = new ArrayList<PolycomPhonebookEntry>();
        dir.transformPhoneBook(Collections.singleton(phonebookEntry), collection);
        PolycomPhonebookEntry entry = collection.iterator().next();
        assertEquals("1234", entry.getFirstName());
        assertNull(entry.getLastName());
        assertEquals("1234", entry.getContact());

    }

    public void testGenerateEmptyDirectory() throws Exception {

        DirectoryConfiguration dir = new DirectoryConfiguration(null, null);

        m_pg.generate(m_location, dir, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-empty-directory.xml");
        Reader expectedXml = new InputStreamReader(expectedPhoneStream);
        Reader generatedXml = m_location.getReader();

        Diff phoneDiff = new Diff(expectedXml, generatedXml);
        assertXMLEqual(phoneDiff, true);
        expectedPhoneStream.close();
    }

    public void testGenerateDirectory() throws Exception {

        PhonebookEntry phonebookEntry = new PhonebookEntry();
        phonebookEntry.setFirstName("Dora");
        phonebookEntry.setLastName("Explorer");
        phonebookEntry.setNumber("210");

        Collection<PhonebookEntry> entries = Collections.singleton(phonebookEntry);
        DirectoryConfiguration dir = new DirectoryConfiguration(entries, null);

        m_pg.generate(m_location, dir, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-directory.xml");
        Reader expectedXml = new InputStreamReader(expectedPhoneStream);
        Reader generatedXml = m_location.getReader();

        Diff phoneDiff = new Diff(expectedXml, generatedXml);
        assertXMLEqual(phoneDiff, true);
        expectedPhoneStream.close();

    }

    public void testGenerateSpeedDialDirectory() throws Exception {
        Button button = new Button();
        button.setLabel("Dora Explorer");
        button.setNumber("210");

        SpeedDial speedDial = new SpeedDial();
        speedDial.setButtons(Collections.singletonList(button));

        DirectoryConfiguration dir = new DirectoryConfiguration(null, speedDial);

        m_pg.generate(m_location, dir, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-speeddial-directory.xml");
        Reader expectedXml = new InputStreamReader(expectedPhoneStream);
        Reader generatedXml = m_location.getReader();

        Diff phoneDiff = new Diff(expectedXml, generatedXml);
        assertXMLEqual(phoneDiff, true);
        expectedPhoneStream.close();
    }
}
