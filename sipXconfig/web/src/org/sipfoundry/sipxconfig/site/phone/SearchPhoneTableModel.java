/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.search.IdentityToBean;
import org.sipfoundry.sipxconfig.search.SearchManager;

/**
 * Geared towards paging-based data models for tapestry TableViews that only load one page at a
 * time and do server-side sorting.
 */
public class SearchPhoneTableModel implements IBasicTableModel {
    private String m_queryText;
    private SearchManager m_searchManager;
    private PhoneContext m_phoneContext;

    public SearchPhoneTableModel(SearchManager searchManager, String queryText,
            PhoneContext phoneContext) {
        m_queryText = queryText;
        m_searchManager = searchManager;
        m_phoneContext = phoneContext;
    }

    public SearchPhoneTableModel() {
        // intentionally empty
    }

    public void setQueryText(String queryText) {
        m_queryText = queryText;
    }

    public void setSearchManager(SearchManager searchManager) {
        m_searchManager = searchManager;
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public int getRowCount() {
        List phones = m_searchManager.search(Phone.class, m_queryText, null);
        return phones.size();
    }

    public Iterator getCurrentPageRows(int firstRow, int pageSize, ITableColumn objSortColumn,
            boolean orderAscending) {
        String[] orderBy = PhoneTableModel.orderByFromSortColum(objSortColumn);
        IdentityToBean identityToBean = new IdentityToBean(m_phoneContext);
        List page = m_searchManager.search(Phone.class, m_queryText, firstRow, pageSize, orderBy,
                orderAscending, identityToBean);
        return page.iterator();
    }
}
