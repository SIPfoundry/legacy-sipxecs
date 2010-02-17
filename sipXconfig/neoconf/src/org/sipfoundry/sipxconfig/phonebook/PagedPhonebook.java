/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.phonebook;

import java.io.Serializable;
import java.util.Collection;
import java.util.LinkedList;

import static java.lang.Integer.parseInt;

public class PagedPhonebook implements Serializable {
    private static final int DEFAULT_START_ROW = 0;
    private static final int DEFAULT_END_ROW = 100;

    private int m_size;
    private int m_startRow;
    private int m_endRow;
    private int m_filteredSize;
    private Collection<PhonebookEntry> m_entries;

    public PagedPhonebook(Collection<PhonebookEntry> entries, int totalSize, String start, String stop) {
        m_size = totalSize;
        m_filteredSize = entries.size();
        m_startRow = parseRow(start, DEFAULT_START_ROW);
        m_endRow = parseRow(stop, DEFAULT_END_ROW);

        if (m_startRow > m_filteredSize) {
            m_startRow = m_filteredSize;
        }

        if (m_endRow > m_filteredSize) {
            m_endRow = m_filteredSize;
        }

        LinkedList<PhonebookEntry> entriesList = new LinkedList<PhonebookEntry>(entries);
        m_entries = entriesList.subList(m_startRow, m_endRow);
    }

    public int getSize() {
        return m_size;
    }

    public int getFilteredSize() {
        return m_filteredSize;
    }

    public int getStartRow() {
        return m_startRow;
    }

    public int getEndRow() {
        return m_endRow;
    }

    public Collection<PhonebookEntry> getEntries() {
        return m_entries;
    }

    private int parseRow(String rowValue, int defaultRow) {
        try {
            return parseInt(rowValue);
        } catch (NumberFormatException ex) {
            return defaultRow;
        }
    }
}
