/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
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
