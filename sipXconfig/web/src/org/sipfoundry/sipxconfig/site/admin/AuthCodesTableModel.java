/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.sipfoundry.sipxconfig.admin.authcode.AuthCode;

public class AuthCodesTableModel implements IBasicTableModel {
    private List<AuthCode> m_codes;

    @Override
    public Iterator getCurrentPageRows(int first, int pageSize, ITableColumn sortColumn, boolean sortOrder) {
        if (m_codes == null) {
            return Collections.emptyList().iterator();
        }

        return m_codes.iterator();
    }

    @Override
    public int getRowCount() {
        if (m_codes == null) {
            return 0;
        }
        return m_codes.size();
    }

    public void setAuthCodes(List<AuthCode> codes) {
        m_codes = codes;
    }

}
