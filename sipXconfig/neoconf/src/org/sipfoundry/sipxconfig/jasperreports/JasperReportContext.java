/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.jasperreports;

import java.util.List;
import java.util.Map;

import net.sf.jasperreports.engine.JasperPrint;

public interface JasperReportContext {
    String getReportsDirectory();

    String getTmpDirectory();

    JasperPrint getJasperPrint(String jasperPath, Map parameters, List< ? > dataSource);

    void generateHtmlReport(JasperPrint jasperPrint, String htmlFile);

    void generatePdfReport(JasperPrint jasperPrint, String pdfFile);

    void generateCsvReport(JasperPrint jasperPrint, String csvFile);

    void generateXlsReport(JasperPrint jasperPrint, String xlsFile);
}
