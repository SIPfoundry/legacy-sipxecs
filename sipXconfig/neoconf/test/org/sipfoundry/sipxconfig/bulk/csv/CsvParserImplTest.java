/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.bulk.csv;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.collections.Closure;
import org.sipfoundry.sipxconfig.bulk.BulkParser;

public class CsvParserImplTest extends TestCase {
    public static final String[] NAMES = {
        "John", "Ringo", "Conference room", "John", "George"
    };

    public static final String[] DESCRIPTIONS = {
        "Room", "Green", "", "", ""
    };

    public void testParseLineEmpty() {
        CsvParserImpl parser = new CsvParserImpl();
        String[] row = parser.parseLine("");
        assertEquals(0, row.length);
    }

    public void testParseLineNoQuotes() {
        CsvParserImpl parser = new CsvParserImpl();
        String[] row = parser.parseLine("a,bbb,c");
        assertEquals(3, row.length);
        assertEquals("a", row[0]);
        assertEquals("bbb", row[1]);
        assertEquals("c", row[2]);
    }

    public void testParseLineStartWithEmpty() {
        CsvParserImpl parser = new CsvParserImpl();
        String[] row = parser.parseLine(",a,bbb,ccc c");
        assertEquals(4, row.length);
        assertEquals("", row[0]);
        assertEquals("a", row[1]);
        assertEquals("bbb", row[2]);
        assertEquals("ccc c", row[3]);
    }

    public void testParseLineEndWithEmpty() {
        CsvParserImpl parser = new CsvParserImpl();
        String[] row = parser.parseLine("a,bbb,ccc c,");
        assertEquals(4, row.length);
        assertEquals("a", row[0]);
        assertEquals("bbb", row[1]);
        assertEquals("ccc c", row[2]);
        assertEquals("", row[3]);
    }

    public void testParseLineQuotes() {
        CsvParserImpl parser = new CsvParserImpl();
        String[] row = parser.parseLine("a,\"bbb\",c,\"\"");
        assertEquals(4, row.length);
        assertEquals("a", row[0]);
        assertEquals("bbb", row[1]);
        assertEquals("c", row[2]);
        assertEquals("", row[3]);
    }

    public void testParseLineQuotedFieldSeparator() {
        CsvParserImpl parser = new CsvParserImpl();
        String[] row = parser.parseLine("a,\"bb,b\",c");
        assertEquals(3, row.length);
        assertEquals("a", row[0]);
        assertEquals("bb,b", row[1]);
        assertEquals("c", row[2]);
    }

    public void testParseLineQuotedQuote() {
        CsvParserImpl parser = new CsvParserImpl();
        String[] row = parser.parseLine("a,\"b\"bb\",c");
        assertEquals(3, row.length);
        assertEquals("a", row[0]);
        assertEquals("b\"bb", row[1]);
        assertEquals("c", row[2]);
    }

    public void testParse() {
        assertEquals(NAMES.length, DESCRIPTIONS.length);

        BulkParser parser = new CsvParserImpl();
        InputStream cutsheet = getClass().getResourceAsStream("cutsheet.csv");
        final List<String[]> rows = new ArrayList<String[]>();
        Closure add = new Closure() {
            public void execute(Object item) {
                rows.add((String[]) item);
            }
        };
        parser.parse(new InputStreamReader(cutsheet), add);
        // there are 6 rows - we expect that the header row is always skipped
        assertEquals(NAMES.length, rows.size());
        for (int i = 0; i < NAMES.length; i++) {
            Object item = rows.get(i);
            assertTrue("row has to be a String array", item instanceof String[]);
            String[] row = (String[]) item;
            assertEquals(12, row.length);
            assertEquals(DESCRIPTIONS[i], row[11]);
            assertEquals(NAMES[i], row[3]);
        }
    }
}
