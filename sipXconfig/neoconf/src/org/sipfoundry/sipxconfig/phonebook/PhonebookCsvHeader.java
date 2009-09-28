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

import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public abstract class PhonebookCsvHeader implements PhonebookFileEntryHelper {
    private static final Log LOG = LogFactory.getLog(PhonebookCsvHeader.class);
    private final Map<String, Integer> m_header;

    public PhonebookCsvHeader(Map<String, Integer> header) {
        m_header = header;
    }

    protected String getValueForSymbol(String[] row, String symbol) {
        Integer index = getIndexForSymbol(symbol);
        if (index == null) {
            LOG.warn("No value for " + symbol);
            return null;
        }
        if (index < 0 || index > row.length) {
            LOG.error("Invalid index for " + symbol);
            return null;
        }
        return row[index];
    }

    protected Integer getIndexForSymbol(String symbol) {
        return m_header.get(symbol);
    }
}
