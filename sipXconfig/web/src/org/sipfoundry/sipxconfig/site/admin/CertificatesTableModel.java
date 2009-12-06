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
import java.util.Set;

import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.sipfoundry.sipxconfig.admin.CertificateDecorator;

public class CertificatesTableModel implements IBasicTableModel {
    private Set<CertificateDecorator> m_certificates;

    @Override
    public Iterator getCurrentPageRows(int first, int pageSize, ITableColumn sortColumn, boolean sortOrder) {
        if (m_certificates == null) {
            return Collections.emptyList().iterator();
        }

        return m_certificates.iterator();
    }

    @Override
    public int getRowCount() {
        if (m_certificates == null) {
            return 0;
        }
        return m_certificates.size();
    }

    public void setCertificates(Set<CertificateDecorator> certificates) {
        m_certificates = certificates;
    }

}
