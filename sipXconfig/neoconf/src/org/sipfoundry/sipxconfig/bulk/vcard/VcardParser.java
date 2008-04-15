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

import java.io.BufferedReader;
import java.io.IOException;
import java.io.Reader;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.collections.Closure;
import org.apache.commons.lang.StringUtils;

public class VcardParser {
    private static final String DEFAULT_VERSION = "2.1";

    private String m_versionPatternString;
    private String m_namePatternString;
    private String m_phoneV21PatternString;
    private String m_phoneV30PatternString;

    public VcardParser(String telType) {
        m_versionPatternString = "(?i)VERSION:((?:\\d)+(?:\\.\\d)*)";
        m_namePatternString = "(?i)N:([^;]*);([^;]*)(?:;[^;]*){3}";
        m_phoneV21PatternString = "(?i)TEL;(?:[a-zA-Z]+;)*" + telType
                + "(?:;[a-zA-Z]+)*:((?:\\+)?\\d+(?:-)?\\d*)";
        m_phoneV30PatternString = "(?i)TEL;(?:(?:TYPE=[a-zA-Z]+;)*TYPE=" + telType
                + "(?:;TYPE=[a-zA-Z]+)*|TYPE=(?:[a-zA-Z]+,)*" + telType
                + "(?:,[a-zA-Z]+)*):((?:\\+)?\\d+(?:-)?\\d*)";
    }

    public void parse(Reader vcard, Closure closure) {
        List<String[]> entryStringsList = parseFile(vcard);
        for (String[] entryStrings : entryStringsList) {
            closure.execute(entryStrings);
        }
    }

    public List<String[]> parseFile(Reader vcard) {
        List<String[]> result = new ArrayList<String[]>();
        try {
            BufferedReader reader = new BufferedReader(vcard);

            String line = null;
            List<RawPhonebookEntry> rawEntries = new ArrayList<RawPhonebookEntry>();
            RawPhonebookEntry rawEntry = null;
            while ((line = reader.readLine()) != null) {
                if (line.equals("BEGIN:vCard")) {
                    rawEntry = new RawPhonebookEntry();
                } else if (!line.equals("END:vCard") && rawEntry != null) {
                    rawEntry.addLineToEntry(line);
                } else if (rawEntry != null) {
                    rawEntries.add(rawEntry);
                }
            }
            reader.close();
            for (RawPhonebookEntry entry : rawEntries) {
                String[] entryStrings = entry.getEntry();
                if (entryStrings != null) {
                    result.add(entryStrings);
                }
            }
            return result;
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    private class RawPhonebookEntry {
        private List<String> m_rawEntry;

        public RawPhonebookEntry() {
            m_rawEntry = new ArrayList<String>();
        }

        public List<String> getRawEntry() {
            return m_rawEntry;
        }

        public void setRawEntry(List<String> entry) {
            m_rawEntry = entry;
        }

        public void addLineToEntry(String line) {
            m_rawEntry.add(line);
        }

        public String[] getEntry() {
            String firstName = null;
            String lastName = null;
            String phoneNumber = null;
            String version = DEFAULT_VERSION;

            Pattern versionPattern = Pattern.compile(m_versionPatternString);
            Pattern namePattern = Pattern.compile(m_namePatternString);
            Pattern phoneV21Pattern = Pattern.compile(m_phoneV21PatternString);
            Pattern phoneV30Pattern = Pattern.compile(m_phoneV30PatternString);

            for (String line : m_rawEntry) {
                Matcher matcher = versionPattern.matcher(line);
                if (matcher.matches()) {
                    version = matcher.group(1);
                } else {
                    matcher.usePattern(namePattern);
                    if (matcher.matches()) {
                        firstName = matcher.group(2);
                        lastName = matcher.group(1);
                    } else {
                        if (version != null) {
                            if (version.equals(DEFAULT_VERSION)) {
                                matcher.usePattern(phoneV21Pattern);
                            } else if (version.equals("3.0")) {
                                matcher.usePattern(phoneV30Pattern);
                            }
                            if (matcher.matches()) {
                                phoneNumber = matcher.group(1);
                            }
                        }
                    }
                }
            }

            if (StringUtils.isNotEmpty(phoneNumber)
                    && (StringUtils.isNotEmpty(firstName) || StringUtils.isNotEmpty(lastName))) {
                String[] row = {
                    firstName, lastName, phoneNumber
                };
                return row;
            }
            return null;
        }
    }
}
