/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.branch;

import java.util.Iterator;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.branch.BranchManager;
import org.sipfoundry.sipxconfig.search.IdentityToBean;
import org.sipfoundry.sipxconfig.search.SearchManager;

public class BranchTableModel implements IBasicTableModel {
    private final BranchManager m_branchManager;

    private String m_queryText;

    private SearchManager m_searchManager;

    private boolean m_searchMode;

    public BranchTableModel(SearchManager searchManager, BranchManager branchManager) {
        m_searchManager = searchManager;
        m_branchManager = branchManager;
    }

    public boolean isSearchMode() {
        return m_searchMode;
    }

    public void setSearchMode(boolean mode) {
        m_searchMode = mode;
    }

    public String getQueryText() {
        return m_queryText;
    }

    public void setQueryText(String text) {
        m_queryText = text;
    }

    public SearchManager getSearchManager() {
        return m_searchManager;
    }

    public void setSearchManager(SearchManager manager) {
        m_searchManager = manager;
    }

    public int getRowCount() {
        if (!isSearchMode() || StringUtils.isBlank(m_queryText)) {
            return m_branchManager.getBranches().size();
        }

        return m_searchManager.search(Branch.class, m_queryText, null).size();
    }

    public Iterator getCurrentPageRows(int firstRow, int pageSize, ITableColumn objSortColumn, boolean orderAscending) {
        String[] orderBy = objSortColumn != null ? new String[] {
            objSortColumn.getColumnName()
        } : ArrayUtils.EMPTY_STRING_ARRAY;
        if (!isSearchMode() || StringUtils.isBlank(m_queryText)) {
            return m_branchManager.loadBranchesByPage(firstRow, pageSize, orderBy, orderAscending).iterator();
        }
        IdentityToBean identityToBean = new IdentityToBean(m_branchManager);
        return m_searchManager.search(Branch.class, m_queryText, firstRow, pageSize, orderBy, orderAscending,
                identityToBean).iterator();
    }
}
