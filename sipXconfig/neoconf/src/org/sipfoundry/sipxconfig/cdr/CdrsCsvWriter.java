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

import org.sipfoundry.sipxconfig.bulk.csv.CsvWriter;
import org.sipfoundry.sipxconfig.cdr.CdrManagerImpl.ColumnInfo;
import org.sipfoundry.sipxconfig.cdr.CdrManagerImpl.ColumnInfoFactory;

class CdrsCsvWriter extends CdrsWriter {
    private CsvWriter m_csv;

    public CdrsCsvWriter(Writer writer, ColumnInfoFactory ciFactory) {
        super(writer, ciFactory);
        m_csv = new CsvWriter(writer);
    }

    protected void writeHeader() throws IOException {
        m_csv.write(ColumnInfo.FIELDS, false);
    }

    protected void writeCdr(ResultSet rs, ColumnInfo[] columns) throws IOException, SQLException {
        String[] row = new String[columns.length];
        for (int i = 0; i < row.length; i++) {
            row[i] = columns[i].formatValue(rs);
        }
        m_csv.write(row, true);
    }
}
