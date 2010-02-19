/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.conference;

import java.util.Iterator;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.BridgeConferenceIdentity;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.search.IdentityToBean;

public class ConferencesTableModel implements IBasicTableModel {
    private final ConferenceBridgeContext m_conferenceContext;

    private Integer m_groupId;

    private final Bridge m_bridge;

    private String m_queryText;

    private BridgeConferenceIdentity m_identity;

    private boolean m_searchMode;

    public ConferencesTableModel(ConferenceBridgeContext conferenceContext, Bridge bridge) {
        this.m_conferenceContext = conferenceContext;
        this.m_bridge = bridge;
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

    public Integer getGroupId() {
        return m_groupId;
    }

    public void setGroupId(Integer id) {
        m_groupId = id;
    }

    public void setQueryText(String text) {
        m_queryText = text;
    }

    public void setIdentity(BridgeConferenceIdentity identity) {
        m_identity = identity;
    }

    public int getRowCount() {
        if (!isSearchMode() || StringUtils.isBlank(m_queryText)) {
            return m_conferenceContext.countFilterConferences(m_bridge.getId(), m_groupId);
        }

        return m_conferenceContext.searchConferences(m_queryText).size();
    }

    public Iterator getCurrentPageRows(int firstRow, int pageSize, ITableColumn objSortColumn, boolean orderAscending) {
        String[] orderBy = objSortColumn != null ? new String[] {
            objSortColumn.getColumnName()
        } : ArrayUtils.EMPTY_STRING_ARRAY;
        if (!isSearchMode() || StringUtils.isBlank(m_queryText)) {
            return m_conferenceContext.filterConferencesByPage(m_bridge.getId(), m_groupId, firstRow, pageSize,
                    orderBy, orderAscending).iterator();
        }
        IdentityToBean identityToBean = new IdentityToBean(m_identity);
        return m_conferenceContext.searchConferences(m_queryText).iterator();
    }

}
