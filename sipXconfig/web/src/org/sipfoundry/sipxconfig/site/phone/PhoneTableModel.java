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

import org.apache.commons.lang.ArrayUtils;
import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

/**
 * Geared towards paging-based data models for tapestry TableViews that only load one page at a
 * time and do server-side sorting.
 */
public class PhoneTableModel implements IBasicTableModel {
    private PhoneContext m_phoneContext;
    private Integer m_groupId;
    private Integer m_branchId;

    public PhoneTableModel(PhoneContext phoneContext, Integer groupId, Integer branchId) {
        m_phoneContext = phoneContext;
        m_groupId = groupId;
        m_branchId = branchId;
    }

    public PhoneTableModel() {
        // intentionally empty
    }

    public void setGroupId(Integer groupId) {
        m_groupId = groupId;
    }

    public void setBranchId(Integer branchId) {
        m_branchId = branchId;
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
        String[] orderBy = orderByFromSortColum(objSortColumn);
        List page = m_phoneContext.loadPhonesByPage(m_groupId, m_branchId, firstRow, pageSize, orderBy,
                orderAscending);
        return page.iterator();
    }

    /**
     * Translates table column to array of phone properties. It is safe to call with null - emtpy
     * array is returned in such case
     *
     * HACK: this is dangerously dependend on relation between the table column name and the
     * properties names
     *
     * @param objSortColumn column object
     * @return array of strings by which we need to sort the table
     */
    public static String[] orderByFromSortColum(ITableColumn objSortColumn) {
        if (objSortColumn == null) {
            return ArrayUtils.EMPTY_STRING_ARRAY;
        }

        String[] orderBy = new String[] {
            objSortColumn.getColumnName()
        };

        // fix for modelId case
        if ("modelId".equals(orderBy[0])) {
            return new String[] {
                "beanId", orderBy[0]
            };
        }

        return orderBy;
    }
}
