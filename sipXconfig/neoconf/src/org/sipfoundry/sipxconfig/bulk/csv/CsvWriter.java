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

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookWriter;

public class CsvWriter implements PhonebookWriter {
    public static final String FORMAT_CSV = "csv";
    private Writer m_writer;
    private boolean m_quote = true;

    public CsvWriter(Writer writer) {
        m_writer = writer;
    }

    public CsvWriter(Writer writer, boolean quote, String[] header) throws IOException {
        m_writer = writer;
        m_quote = quote;
        write(header, m_quote);
    }

    public void write(String[] fields, boolean quote) throws IOException {
        StringBuilder line = new StringBuilder();
        for (int i = 0; i < fields.length; i++) {
            String field = fields[i];

            if (quote) {
                line.append(CsvParserImpl.FIELD_QUOTE);
            }
            line.append(StringUtils.defaultString(field));
            if (quote) {
                line.append(CsvParserImpl.FIELD_QUOTE);
            }
            if (i < fields.length - 1) {
                line.append(CsvParserImpl.FIELD_SEPARATOR);
            } else {
                line.append('\n');
            }
        }
        m_writer.write(line.toString());
    }

    public void write(PhonebookEntry entry) throws IOException {
        if (entry == null) {
            return;
        }
        if (!entry.isWritable()) {
            return;
        }
        write(entry.getFields(), m_quote);
    }

    /**
     * Similar to write but translates exceptions to UserException
     */
    public void optimisticWrite(String[] fields, boolean quote) {
        try {
            write(fields, quote);
        } catch (IOException e) {
            new RuntimeException(e);
        }
    }

}
