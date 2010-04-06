/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.bulk.encoding;

import java.io.InputStream;

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManagerImpl;

public class EncodingTest  extends TestCase {
    public void testGetEncoding() throws Exception {
        PhonebookManagerImpl phonebookManager = new PhonebookManagerImpl();

        InputStream fileStream1 = getClass().getResourceAsStream("phonebook1.vcf");
        assertEquals("UTF-8", phonebookManager.getEncoding(fileStream1));

        InputStream fileStream2 = getClass().getResourceAsStream("phonebook2.csv");
        assertTrue(phonebookManager.getEncoding(fileStream2).startsWith("UTF-16"));

        InputStream fileStream4 = getClass().getResourceAsStream("phonebook3.vcf");
        assertEquals("US-ASCII", phonebookManager.getEncoding(fileStream4));
    }

}
