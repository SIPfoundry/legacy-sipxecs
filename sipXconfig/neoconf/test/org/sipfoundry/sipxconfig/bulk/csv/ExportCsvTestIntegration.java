/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.bulk.csv;

import java.io.InputStream;
import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.common.CoreContextImpl;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class ExportCsvTestIntegration extends IntegrationTestCase {

    private ExportCsv m_exportCsv;
    private CoreContextImpl m_coreContext;
    private DomainManager m_originalDomainManager;

    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
    }

    @Override
    protected void onTearDownInTransaction() throws Exception {
        super.onTearDownInTransaction();
        m_coreContext.setDomainManager(m_originalDomainManager);
    }

    public void setExportCsv(ExportCsv exportCsv) {
        m_exportCsv = exportCsv;
    }

    public void setCoreContextImpl(CoreContextImpl coreContextImpl) {
        m_coreContext = coreContextImpl;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_originalDomainManager = domainManager;
    }

    public void testExport() throws Exception {
        loadDataSet("common/TestUserSeed.db.xml");
        loadDataSet("phone/PhoneSummarySeed.xml");

        StringWriter writer = new StringWriter();
        m_exportCsv.exportCsv(writer);

        InputStream expectedProfile = getClass().getResourceAsStream("export.csv");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);

        assertEquals(expected, writer.toString());
    }
}
