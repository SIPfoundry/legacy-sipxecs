/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.bulk.csv;

import java.io.IOException;
import java.io.Writer;

import org.apache.commons.lang.StringUtils;

public class SimpleCsvWriter {
    private Writer m_writer;
    private boolean m_quote = true;
    private char m_delim = CsvParserImpl.FIELD_SEPARATOR;

    public SimpleCsvWriter(Writer writer) {
        m_writer = writer;
    }

    public SimpleCsvWriter(Writer writer, boolean quote) throws IOException {
        m_writer = writer;
        m_quote = quote;
    }

    public SimpleCsvWriter(Writer writer, boolean quote, String[] header) throws IOException {
        m_writer = writer;
        m_quote = quote;
        write(header);
    }

    public void write(String[] fields, boolean quote) throws IOException {
        StringBuilder line = new StringBuilder();
        for (int i = 0; i < fields.length; i++) {
            String field = fields[i];

            if (quote) {
                line.append(CsvParserImpl.FIELD_QUOTE);
            }
            line.append(StringUtils.defaultString(field).replace("\"", "\\\""));
            if (quote) {
                line.append(CsvParserImpl.FIELD_QUOTE);
            }
            if (i < fields.length - 1) {
                line.append(m_delim);
            } else {
                line.append('\n');
            }
        }
        m_writer.write(line.toString());
    }

    public void write(String[] fields) throws IOException {
        write(fields, m_quote);
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

    protected Writer getWriter() {
        return m_writer;
    }

    public boolean isQuote() {
        return m_quote;
    }

    public void setQuote(boolean quote) {
        m_quote = quote;
    }

    public void setDelim(char delim) {
        m_delim = delim;
    }
}
