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

import java.io.BufferedReader;
import java.io.IOException;
import java.io.Reader;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.collections.Closure;
import org.sipfoundry.sipxconfig.bulk.BulkParser;

/**
 * Very simple CSV parser. Always skips headers, field quote is optional and only matters on field
 * boundaries. Field separator is ignored inside of field quote
 *
 * TODO: Switch to commons CSV parser once it's available
 */
public class CsvParserImpl implements BulkParser {

    static final char FIELD_SEPARATOR = ',';
    static final char FIELD_QUOTE = '"';

    /**
     * it's only used as initialization of ArrayList - the parses will well with more fields as
     * well
     */
    private static final int DEFAULT_FIELD_COUNT = 32;

    private boolean m_skipHeaderLine = true;

    public List parse(Reader csv) {
        final List result = new ArrayList();
        Closure resultAdder = new Closure() {
            public void execute(Object input) {
                result.add(input);
            }
        };
        parse(csv, resultAdder);
        return result;
    }

    /**
     * This method reads the Reader line by line and gets rid of the unwanted line breaks that
     * outlook might introduce
     */
    public void parse(Reader csv, Closure closure) {
        try {
            BufferedReader reader = new BufferedReader(csv);
            CharSequence line;
            if (m_skipHeaderLine) {
                // skip header
                line = reader.readLine();
            }
            // read remaining file
            while ((line = readLine(reader)) != null) {
                String[] row = parseLine(line);
                closure.execute(row);
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    protected String[] parseLine(CharSequence line) {
        ArrayList row = new ArrayList(DEFAULT_FIELD_COUNT);

        boolean inQuotedField = false;
        int fieldStart = 0;

        final int len = line.length();
        for (int i = 0; i < len; i++) {
            char c = line.charAt(i);
            if (c == FIELD_SEPARATOR) {
                if (!inQuotedField) {
                    // ignore we are quoting
                    addField(row, line, fieldStart, i);
                    fieldStart = i + 1;
                }
            } else if (c == FIELD_QUOTE) {
                if (inQuotedField) {
                    // we are already quoting - peek to see if this is the end of the field
                    if (i + 1 == len || line.charAt(i + 1) == FIELD_SEPARATOR) {
                        addField(row, line, fieldStart, i);
                        fieldStart = i + 2;
                        // and skip the comma
                        i++;
                        inQuotedField = false;
                    }
                } else if (fieldStart == i) {
                    // this is a beginning of a quote
                    inQuotedField = true;
                    // move field start
                    fieldStart++;
                }
            }
        }
        // add last field - but only if string was not empty
        if (len > 0 && fieldStart <= len) {
            addField(row, line, fieldStart, len);
        }

        return (String[]) row.toArray(new String[row.size()]);
    }


    /**
     * Special version of readLine - it takes into account the fact that CSV fields can contain /n
     * characters. It read lines until all the CSV fields are closed.
     *
     * @param reader CSV content
     * @return line or null if no more, line can have multiple \n characters in it
     */
    protected CharSequence readLine(BufferedReader reader) throws IOException {
        String line;
        StringBuilder sb = new StringBuilder();
        while ((line = reader.readLine()) != null) {
            sb.append(line);
            if (!hasUnmatchedFieldQuotes(sb)) {
                break;
            }
            sb.append('\n');
        }
        if (line == null && sb.length() == 0) {
            return null;
        }
        return sb;
    }

    /**
     * Checks if the line has contains full CSV fields only.
     *
     * @return true if String contains unmatched quote characters
     */
    boolean hasUnmatchedFieldQuotes(CharSequence line) {
        boolean inQuotedField = false;
        boolean possibleEndQuote = false;
        for (int i = 0; i < line.length(); i++) {
            if (FIELD_QUOTE == line.charAt(i)) {
                if (inQuotedField) {
                    possibleEndQuote = true;
                } else {
                    inQuotedField = true;
                }
            } else if (possibleEndQuote) {
                if (FIELD_SEPARATOR == line.charAt(i)) {
                    inQuotedField = false;
                }
                possibleEndQuote = false;
            }
        }
        if (possibleEndQuote) {
            inQuotedField = false;
        }
        return inQuotedField;
    }

    private void addField(List row, CharSequence line, int startIndex, int endIndex) {
        String field = line.subSequence(startIndex, endIndex).toString();
        row.add(field);
    }

    public void setSkipHeaderLine(boolean skipHeaderLine) {
        m_skipHeaderLine = skipHeaderLine;
    }
}
