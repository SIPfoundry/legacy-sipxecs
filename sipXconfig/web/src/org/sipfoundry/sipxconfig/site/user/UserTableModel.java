/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.sipfoundry.sipxconfig.common.CoreContext;

public class UserTableModel implements IBasicTableModel {
    private CoreContext m_coreContext;
    private Integer m_groupId;
    private Integer m_branchId;
    private String m_searchString;

    public UserTableModel(CoreContext coreContext, Integer groupId, Integer branchId, String searchString) {
        setCoreContext(coreContext);
        setGroupId(groupId);
        setBranchId(branchId);
        setSearchString(searchString);
    }

    public UserTableModel() {
        // intentionally empty
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public int getRowCount() {
        int count = m_coreContext.getUsersInGroupWithSearchCount(m_groupId, m_searchString);
        return count;
    }

    public Integer getGroupId() {
        return m_groupId;
    }

    public void setGroupId(Integer groupId) {
        m_groupId = groupId;
    }

    public Integer getBranchId() {
        return m_branchId;
    }

    public void setBranchId(Integer branchId) {
        m_branchId = branchId;
    }

    public String getSearchString() {
        return m_searchString;
    }

    public void setSearchString(String searchString) {
        m_searchString = searchString;
    }

    public Iterator getCurrentPageRows(int firstRow, int pageSize, ITableColumn objSortColumn,
            boolean orderAscending) {
        String orderBy = objSortColumn != null ? objSortColumn.getColumnName() : null;
        List page = m_coreContext.loadUsersByPage(m_searchString, m_groupId, m_branchId, firstRow, pageSize, orderBy,
                orderAscending);
        return page.iterator();
    }
}
