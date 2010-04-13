/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.cdr;

import java.util.Date;
import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.sipfoundry.sipxconfig.cdr.Cdr;
import org.sipfoundry.sipxconfig.cdr.CdrManager;
import org.sipfoundry.sipxconfig.cdr.CdrSearch;
import org.sipfoundry.sipxconfig.common.User;

public class CdrTableModel implements IBasicTableModel {
    private CdrManager m_cdrManager;
    private CdrSearch m_cdrSearch;
    private User m_activeUser;
    private Date m_from;
    private Date m_to;

    public CdrTableModel(CdrManager cdrManager) {
        m_cdrManager = cdrManager;
    }

    public void setCdrSearch(CdrSearch cdrSearch) {
        m_cdrSearch = cdrSearch;
    }

    /**
     * Set the active user, if this is being called to display a user's CDR
     * records
     * @param user
     */
    public void setUser(User user) {
        m_activeUser = user;
    }


    public void setFrom(Date from) {
        m_from = from;
    }

    public void setTo(Date to) {
        m_to = to;
    }

    public Iterator getCurrentPageRows(int nFirst, int nPageSize, ITableColumn objSortColumn,
            boolean ascending) {
        if (objSortColumn != null) {
            m_cdrSearch.setOrder(objSortColumn.getColumnName(), ascending);
        }
        List<Cdr> cdrs = m_cdrManager.getCdrs(m_from, m_to, m_cdrSearch,
                m_activeUser, nPageSize, nFirst);
        return cdrs.iterator();
    }

    public int getRowCount() {
        return m_cdrManager.getCdrCount(m_from, m_to, m_cdrSearch, m_activeUser);
    }
}
