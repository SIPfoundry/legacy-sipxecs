/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.test;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;

import org.springframework.jdbc.core.RowCallbackHandler;

public class ResultDataGrid implements RowCallbackHandler {
    private List<Object[]> m_results = new ArrayList<Object[]>();

    public int getRowCount() {
        return m_results.size();
    }

    @Override
    public void processRow(ResultSet arg0) throws SQLException {
        int count = arg0.getMetaData().getColumnCount();
        Object[] cols = new Object[count];
        for (int i = 0; i < count; i++) {
            cols[i] = arg0.getObject(i + 1);
        }
        m_results.add(cols);
    }

    public Object[][] toArray() {
        return m_results.toArray(new Object[0][0]);
    }
}
