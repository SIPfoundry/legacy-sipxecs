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

import java.io.StringWriter;

import junit.framework.TestCase;

public class CsvWriterTest extends TestCase {

    protected void setUp() throws Exception {
        super.setUp();
    }

    public void testWriteNoQuote() throws Exception {
        StringWriter writer = new StringWriter();
        CsvWriter csvWriter = new CsvWriter(writer);
        String[] row = {
            "aa", null, "cc"
        };
        csvWriter.write(row, false);
        assertEquals("aa,,cc\n", writer.getBuffer().toString());
    }

    public void testWriteQuote() throws Exception {
        StringWriter writer = new StringWriter();
        CsvWriter csvWriter = new CsvWriter(writer);
        String[] row = {
            "aa", null, "cc"
        };
        csvWriter.write(row, true);
        assertEquals("\"aa\",\"\",\"cc\"\n", writer.getBuffer().toString());
    }
}
