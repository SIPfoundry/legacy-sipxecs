/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.bulk.vcard;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.Reader;
import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.test.TestUtil;

public class VcardParserTest extends TestCase {

    public void setUp() {
    }

    public void testParse() {

        try {
            File testPhonebookFile = new File(TestUtil.getTestSourceDirectory(getClass()) + "/" + "testPhonebook.vcf");
            Reader reader = new FileReader(testPhonebookFile);
            VcardParser parser = new VcardParser("work");
            List<String[]> entriesList = parser.parseFile(reader);
            assertEquals(2, entriesList.size());

            String[] entry1 = entriesList.get(0);
            assertEquals(3, entry1.length);
            assertEquals("Jean Luc", entry1[0]);
            assertEquals("Picard", entry1[1]);
            assertEquals("1234",entry1[2]);

            String[] entry2 = entriesList.get(1);
            assertEquals(3, entry2.length);
            assertEquals("Luke", entry2[0]);
            assertEquals("Skywalker", entry2[1]);
            assertEquals("1235",entry2[2]);
        } catch (FileNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }
}
