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
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

/**
 * Geared towards paging-based data models for tapestry TableViews that only load one page at a
 * time and do server-side sorting.
 */
public class PhoneTableModel implements IBasicTableModel {
    private PhoneContext m_phoneContext;
    private Integer m_groupId;

    public PhoneTableModel(PhoneContext phoneContext, Integer groupId) {
        m_phoneContext = phoneContext;
        m_groupId = groupId;
    }

    public PhoneTableModel() {
        // intentionally empty
    }

    public void setGroupId(Integer groupId) {
        m_groupId = groupId;
    }

    public void setPhoneContext(PhoneContext context) {
        m_phoneContext = context;
    }

    public PhoneContext getPhoneContext() {
        return m_phoneContext;
    }

    public int getRowCount() {
        return m_phoneContext.getPhonesInGroupCount(m_groupId);
    }

    public Iterator getCurrentPageRows(int firstRow, int pageSize, ITableColumn objSortColumn,
            boolean orderAscending) {
        String[] orderBy = TapestryUtils.orderByFromSortColum(objSortColumn);
        List page = m_phoneContext.loadPhonesByPage(m_groupId, firstRow, pageSize, orderBy,
                orderAscending);
        return page.iterator();
    }
}
