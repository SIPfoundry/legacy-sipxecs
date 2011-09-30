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

import java.io.IOException;
import java.io.Writer;

import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookWriter;

public class CsvWriter extends SimpleCsvWriter implements PhonebookWriter {

    public CsvWriter(Writer writer) {
        super(writer);
    }

    public CsvWriter(Writer writer, boolean quote) throws IOException {
        super(writer, quote);
    }

    public CsvWriter(Writer writer, boolean quote, String[] header) throws IOException {
        super(writer, quote, header);
    }

    public void write(PhonebookEntry entry) throws IOException {
        if (entry == null) {
            return;
        }
        if (!entry.isWritable()) {
            return;
        }
        write(entry.getFields());
    }

}
