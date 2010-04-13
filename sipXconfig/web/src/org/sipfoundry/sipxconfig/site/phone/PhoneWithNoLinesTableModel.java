/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

/**
 * Geared towards paging-based data models for tapestry TableViews that only load one page at a
 * time and do server-side sorting.
 */
public class PhoneWithNoLinesTableModel extends PhoneTableModel {

    public PhoneWithNoLinesTableModel(PhoneContext phoneContext) {
        super(phoneContext, null);
    }

    public int getRowCount() {
        return getPhoneContext().getPhonesWithNoLinesCount();
    }

    public Iterator getCurrentPageRows(int firstRow, int pageSize, ITableColumn objSortColumn,
            boolean orderAscending) {
        String[] orderBy = orderByFromSortColum(objSortColumn);
        List page = getPhoneContext().loadPhonesWithNoLinesByPage(firstRow, pageSize, orderBy,
                orderAscending);
        return page.iterator();
    }
}
