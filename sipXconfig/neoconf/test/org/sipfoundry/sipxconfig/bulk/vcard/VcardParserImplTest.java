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

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.collections.Closure;

public class VcardParserImplTest extends TestCase {

    public void testParse() throws Exception {

        InputStream testPhonebookFile = getClass().getResourceAsStream("testPhonebook.vcf");
        Reader reader = new InputStreamReader(testPhonebookFile);
        VcardParserImpl parser = new VcardParserImpl();
        parser.setTelType("work");
        final List<String[]> entriesList = new ArrayList<String[]>();
        Closure add = new Closure() {
            public void execute(Object item) {
                entriesList.add((String[]) item);
            }
        };

        parser.parse(reader, add);
        assertEquals(6, entriesList.size());

        String[] entry1 = entriesList.get(0);
        assertEquals(3, entry1.length);
        assertEquals("34 Jean 2Luc4", entry1[0]);
        assertEquals("Picard12", entry1[1]);
        assertEquals("1234", entry1[2]);

        String[] entry2 = entriesList.get(1);
        assertEquals(3, entry2.length);
        assertEquals("Luke", entry2[0]);
        assertEquals("Skywalker", entry2[1]);
        assertEquals("1235", entry2[2]);

        String[] entry3 = entriesList.get(2);
        assertEquals(3, entry3.length);
        assertEquals("Venus", entry3[0]);
        assertEquals("Williams", entry3[1]);
        assertEquals("(998) 678-5667", entry3[2]);

        String[] entry4 = entriesList.get(3);
        assertEquals(3, entry4.length);
        assertEquals("Cindrella", entry4[0]);
        assertEquals("", entry4[1]);
        assertEquals("+1-213-555-1234", entry4[2]);

        String[] entry5 = entriesList.get(4);
        assertEquals(3, entry5.length);
        assertEquals("Micha3l", entry5[0]);
        assertEquals("Jackson", entry5[1]);
        assertEquals("+40 (21)313 17 -98", entry5[2]);

        String[] entry6 = entriesList.get(5);
        assertEquals(3, entry6.length);
        assertEquals("John", entry6[0]);
        assertEquals("Mc'Donald", entry6[1]);
        assertEquals("ESN-8776", entry6[2]);

    }
}
