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
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.collections.Closure;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.bulk.BulkParser;

public class VcardParserImpl implements BulkParser {
    private static final String DEFAULT_VERSION = "2.1";
    private Pattern m_versionPattern = Pattern.compile("(?i)VERSION:((?:\\d)+(?:\\.\\d)*)");
    private Pattern m_namePattern = Pattern.compile("(?i)N:([^;]*);([^;]*)(?:;[^;]*){3}");
    private Pattern m_phoneV21Pattern;
    private Pattern m_phoneV30Pattern;

    public void setTelType(String telType) {
        String phoneV21PatternString = "(?i)TEL;(?:[a-zA-Z]+;)*" + telType + "(?:;[a-zA-Z]+)*:((?:\\+)?\\d+(?:-)?\\d*)";
        String phoneV30PatternString = "(?i)TEL;(?:(?:TYPE=[a-zA-Z]+;)*TYPE=" + telType
                + "(?:;TYPE=[a-zA-Z]+)*|TYPE=(?:[a-zA-Z]+,)*" + telType + "(?:,[a-zA-Z]+)*):((?:\\+)?\\d+(?:-)?\\d*)";
        m_phoneV21Pattern = Pattern.compile(phoneV21PatternString);
        m_phoneV30Pattern = Pattern.compile(phoneV30PatternString);
    }

    public void parse(Reader vcard, Closure closure) {
        BufferedReader reader = new BufferedReader(vcard);
        try {
            RawPhonebookEntry entry = null;
            String line = null;
            while ((line = reader.readLine()) != null) {
                if (line.equals("BEGIN:vCard")) {
                    entry = new RawPhonebookEntry();
                    continue;
                }
                if (entry == null) {
                    continue;
                }
                if (!line.equals("END:vCard")) {
                    entry.addLineToEntry(line);
                } else {
                    String[] entryStrings = entry.getEntry();
                    if (entryStrings != null) {
                        closure.execute(entryStrings);
                    }
                    entry = null;
                }
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(reader);
        }
    }

    private class RawPhonebookEntry {
        private String m_firstName;
        private String m_lastName;
        private String m_phoneNumber;
        private String m_version;

        public void addLineToEntry(String line) {
            Matcher matcher = m_versionPattern.matcher(line);
            if (matcher.matches()) {
                m_version = matcher.group(1);
                return;
            }

            matcher.usePattern(m_namePattern);
            if (matcher.matches()) {
                m_firstName = matcher.group(2);
                m_lastName = matcher.group(1);
                return;
            }

            if (m_version != null) {
                if (m_version.equals(DEFAULT_VERSION)) {
                    matcher.usePattern(m_phoneV21Pattern);
                } else if (m_version.equals("3.0")) {
                    matcher.usePattern(m_phoneV30Pattern);
                }
                if (matcher.matches()) {
                    m_phoneNumber = matcher.group(1);
                    return;
                }
            }
        }

        public String[] getEntry() {
            if (isEmpty()) {
                return null;
            }
            return new String[] {
                m_firstName, m_lastName, m_phoneNumber
            };
        }

        private boolean isEmpty() {
            return StringUtils.isEmpty(m_phoneNumber)
                    || (StringUtils.isEmpty(m_firstName) && StringUtils.isEmpty(m_lastName));
        }
    }
}
