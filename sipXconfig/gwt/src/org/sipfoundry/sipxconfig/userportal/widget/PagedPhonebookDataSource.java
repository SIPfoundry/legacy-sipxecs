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

public class PagedPhonebookDataSource extends PhonebookDataSource {

    public PagedPhonebookDataSource(String id) {
        super(id);
        setRecordXPath("/phonebook/entries/entry");
        setDataURL("/sipxconfig/rest/my/pagedphonebook");
    }

    @Override
    protected Object transformRequest(DSRequest request) {

        if (request.getOperationType().equals(DSOperationType.FETCH)) {
            request.setActionURL(getDataURL() + "?start=" + request.getStartRow() + "&end=" + request.getEndRow()
                    + "&filter=" + request.getCriteria().getAttributeAsString(QUERY));
        }

        return super.transformRequest(request);
    }

    @Override
    protected void transformResponse(DSResponse response, DSRequest request, Object data) {

        if (request.getOperationType().equals(DSOperationType.FETCH)) {
            Integer size = Integer.parseInt(XMLTools.selectString(data, "/phonebook/size"));
            Integer filteredSize = Integer.parseInt(XMLTools.selectString(data, "/phonebook/filtered-size"));
            Integer startRow = Integer.parseInt(XMLTools.selectString(data, "/phonebook/start-row"));
            Integer endRow = Integer.parseInt(XMLTools.selectString(data, "/phonebook/end-row"));

            response.setTotalRows(filteredSize);
            response.setStartRow(startRow);
            response.setEndRow(endRow);
            onDataFetch(size, filteredSize);
        } else {
            super.transformResponse(response, request, data);
        }

    }

    protected void onDataFetch(Integer size, Integer filteredSize) {

    }
}
