/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.InputStream;
import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.IntegrationTestCase;

public class ExportCsvTestIntegration extends IntegrationTestCase {

    private ExportCsv m_exportCsv;

    public void setExportCsv(ExportCsv exportCsv) {
        m_exportCsv = exportCsv;
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
