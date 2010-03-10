/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.userportal.widget;

import com.smartgwt.client.data.DSRequest;
import com.smartgwt.client.data.DSResponse;
import com.smartgwt.client.data.XMLTools;
import com.smartgwt.client.types.DSOperationType;

public abstract class PagedPhonebookDataSource extends PhonebookDataSource {

    public static final int PAGE_SIZE = 500;
    private int m_pageNumber;

    public PagedPhonebookDataSource(String id) {
        super(id);
        setRecordXPath("/phonebook/entries/entry");
        setDataURL("/sipxconfig/rest/my/pagedphonebook");
        setPageNumber(1);
    }

    public void setPageNumber(int pageNumber) {
        m_pageNumber = pageNumber;
    }

    public int getPageNumber() {
        return m_pageNumber;
    }

    @Override
    protected Object transformRequest(DSRequest request) {

        if (request.getOperationType().equals(DSOperationType.FETCH)) {
            String startRow = String.valueOf((PAGE_SIZE * getPageNumber()) - PAGE_SIZE);
            String endRow = String.valueOf(PAGE_SIZE * getPageNumber());
            request.setActionURL(getDataURL() + "?start=" + startRow + "&end=" + endRow
                    + "&filter=");
        }

        return super.transformRequest(request);
    }

    @Override
    protected void transformResponse(DSResponse response, DSRequest request, Object data) {
        if (request.getOperationType().equals(DSOperationType.FETCH)) {
            Integer size = Integer.parseInt(XMLTools.selectString(data, "/phonebook/size"));
            String showOnPhoneStatus = XMLTools.selectString(data, "/phonebook/show-on-phone");
            String googleDomain = XMLTools.selectString(data, "/phonebook/google-domain");
            onDataFetch(size, showOnPhoneStatus, googleDomain);
        } else {
            super.transformResponse(response, request, data);
        }
    }

    protected abstract void onDataFetch(Integer size, String showOnPhoneStatus, String googleDomain);
}
