/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.sipfoundry.sipxconfig.admin.tls.TlsPeer;

public class TlsPeersTableModel implements IBasicTableModel {
    private List<TlsPeer> m_peers;

    @Override
    public Iterator getCurrentPageRows(int first, int pageSize, ITableColumn sortColumn, boolean sortOrder) {
        if (m_peers == null) {
            return Collections.emptyList().iterator();
        }

        return m_peers.iterator();
    }

    @Override
    public int getRowCount() {
        if (m_peers == null) {
            return 0;
        }
        return m_peers.size();
    }

    public void setTlsPeers(List<TlsPeer> peers) {
        m_peers = peers;
    }

}
