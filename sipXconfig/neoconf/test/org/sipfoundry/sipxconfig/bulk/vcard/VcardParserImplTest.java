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
        assertEquals(2, entriesList.size());

        String[] entry1 = entriesList.get(0);
        assertEquals(3, entry1.length);
        assertEquals("Jean Luc", entry1[0]);
        assertEquals("Picard", entry1[1]);
        assertEquals("1234", entry1[2]);

        String[] entry2 = entriesList.get(1);
        assertEquals(3, entry2.length);
        assertEquals("Luke", entry2[0]);
        assertEquals("Skywalker", entry2[1]);
        assertEquals("1235", entry2[2]);
    }
}
