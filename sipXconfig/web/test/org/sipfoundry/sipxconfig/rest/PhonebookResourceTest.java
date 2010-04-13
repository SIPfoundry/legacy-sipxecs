/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.rest;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Collections;

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.dom4j.Document;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.rest.PhonebookResource.PhonebookCsv;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class PhonebookResourceTest extends TestCase {
    private PhonebookEntry m_entry;

    @Override
    protected void setUp() throws Exception {
        m_entry = new PhonebookEntry();
        m_entry.setFirstName("Penguin");
        m_entry.setLastName("Emporer");
        m_entry.setNumber("happyfeet@southpole.org");
    }

    public void testCsv() throws IOException {
        ByteArrayOutputStream actual = new ByteArrayOutputStream();
        PhonebookCsv csv = new PhonebookCsv(Collections.singleton(m_entry));
        csv.write(actual);

        assertEquals("\"First name\",\"Last name\",\"Number\"\n"
                + "\"Penguin\",\"Emporer\",\"happyfeet@southpole.org\"\n", actual.toString());
    }

    public void testXml() throws Exception {
        Document phonebookXml = PhonebookResource.getPhonebookXml(Collections.singleton(m_entry));

        String phonebook = TestUtil.asString(phonebookXml);
        String expected = IOUtils.toString(getClass().getResourceAsStream("phonebook.test.xml"));
        assertEquals(expected, phonebook);
    }
}
