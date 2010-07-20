/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.sipfoundry.sipxconfig.admin.authcode.AuthCode;
import org.sipfoundry.sipxconfig.admin.authcode.AuthCodeManager;
import org.sipfoundry.sipxconfig.search.IdentityToBean;
import org.sipfoundry.sipxconfig.search.SearchManager;
/**
 *
 * Auth Code table model which supports searching functionalities
 *
 */

public class SearchAuthCodesTableModel implements IBasicTableModel {
    private List<AuthCode> m_codes;
    private String m_queryText;
    private SearchManager m_searchManager;
    private AuthCodeManager m_authCodeManager;

    public SearchAuthCodesTableModel(SearchManager searchManager,
            String queryText, AuthCodeManager authCodeManager) {
        m_queryText = queryText;
        m_searchManager = searchManager;
        m_authCodeManager = authCodeManager;
    }

    public SearchAuthCodesTableModel() {
        // intentionally empty
    }

    public void setQueryText(String queryText) {
        m_queryText = queryText;
    }

    public void setSearchManager(SearchManager searchManager) {
        m_searchManager = searchManager;
    }

    public void setAuthCodeManager(AuthCodeManager authCodeManager) {
        m_authCodeManager = authCodeManager;
    }

    @Override
    public int getRowCount() {
        List codes = m_searchManager.search(AuthCode.class, m_queryText, null);
        return codes.size();
    }

    @Override
    public Iterator getCurrentPageRows(int firstRow, int pageSize,
            ITableColumn objSortColumn, boolean orderAscending) {
        //String[] orderBy = PhoneTableModel.orderByFromSortColum(objSortColumn);
        String[] orderBy = {"code", "description"};
        IdentityToBean identityToBean = new IdentityToBean(m_authCodeManager);
        List page = m_searchManager.search(AuthCode.class, m_queryText,
                firstRow, pageSize, orderBy, orderAscending, identityToBean);
        return page.iterator();
    }

    public void setAuthCodes(List<AuthCode> codes) {
        m_codes = codes;
    }

}
