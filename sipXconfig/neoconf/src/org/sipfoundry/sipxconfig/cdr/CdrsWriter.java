/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.cdr;

import java.io.IOException;
import java.io.Writer;
import java.sql.ResultSet;
import java.sql.SQLException;

import org.sipfoundry.sipxconfig.cdr.CdrManagerImpl.ColumnInfo;
import org.sipfoundry.sipxconfig.cdr.CdrManagerImpl.ColumnInfoFactory;
import org.springframework.jdbc.core.RowCallbackHandler;

abstract class CdrsWriter implements RowCallbackHandler {
    private ColumnInfo[] m_columns;
    private ColumnInfoFactory m_ciFactory;

    private Writer m_writer;

    public CdrsWriter(Writer writer, ColumnInfoFactory ciFactory) {
        m_writer = writer;
        m_ciFactory = ciFactory;
    }

    protected void writeHeader() throws IOException {
        // empty by default
    }

    protected void writeFooter() throws IOException {
        // empty by default
    }

    protected abstract void writeCdr(ResultSet rs, ColumnInfo[] columns) throws IOException, SQLException;

    protected final Writer getWriter() {
        return m_writer;
    }

    public final void processRow(ResultSet rs) throws SQLException {
        if (m_columns == null) {
            m_columns = m_ciFactory.create(rs);
        }
        try {
            writeCdr(rs, m_columns);
        } catch (IOException e) {
            new RuntimeException(e);
        }
    }
}
