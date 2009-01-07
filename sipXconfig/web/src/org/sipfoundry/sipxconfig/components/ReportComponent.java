/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import java.io.File;
import java.util.List;
import java.util.Map;

import javax.servlet.http.HttpSession;

import net.sf.jasperreports.engine.JasperPrint;
import net.sf.jasperreports.j2ee.servlets.BaseHttpServlet;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.services.RequestGlobals;
import org.sipfoundry.sipxconfig.jasperreports.JasperReportContext;

@ComponentClass(allowBody = false, allowInformalParameters = true)
public abstract class ReportComponent extends BaseComponent {
    private static final String REPORT_DESIGN_TYPE = ".jrxml";

    private static final String REPORT_JASPER_TYPE = ".jasper";

    private static final String REPORT_HTML_TYPE = ".html";

    private static final String REPORT_PDF_TYPE = ".pdf";

    private static final String REPORT_CSV_TYPE = ".csv";

    private static final String REPORT_XLS_TYPE = ".xls";

    @InjectObject(value = "service:tapestry.globals.RequestGlobals")
    public abstract RequestGlobals getRequestGlobals();

    @InjectObject(value = "spring:jasperReportContextImpl")
    public abstract JasperReportContext getJasperReportContext();

    @Parameter(name = "reportLabel", required = true)
    public abstract String getReportLabel();

    @Parameter(name = "reportName", required = true)
    public abstract String getReportName();

    @Parameter(name = "reportData", required = true)
    public abstract List< ? > getReportData();

    @Parameter(name = "reportParameters", required = true)
    public abstract Map<String, String> getReportParameters();

    @Parameter(name = "showPdfLink", required = false, defaultValue = "true")
    public abstract boolean getShowPdfLink();

    @Parameter(name = "showCsvLink", required = false, defaultValue = "true")
    public abstract boolean getShowCsvLink();

    @Parameter(name = "showXlsLink", required = false, defaultValue = "true")
    public abstract boolean getShowXlsLink();

    public boolean isReportsGenerated() {
        File htmlReport = new File(getHtmlReportPath());
        if (htmlReport.exists()) {
            return true;
        }
        return false;
    }

    public String getDesignReportPath() {
        return getJasperReportContext().getReportsDirectory() + File.separator + getReportName()
                + REPORT_DESIGN_TYPE;
    }

    public String getHtmlReportPath() {
        return getJasperReportContext().getTmpDirectory() + File.separator + getReportName()
                + REPORT_HTML_TYPE;
    }

    public String getPdfReportPath() {
        return getJasperReportContext().getTmpDirectory() + File.separator + getReportName()
                + REPORT_PDF_TYPE;
    }

    public String getCsvReportPath() {
        return getJasperReportContext().getTmpDirectory() + File.separator + getReportName()
                + REPORT_CSV_TYPE;
    }

    public String getXlsReportPath() {
        return getJasperReportContext().getTmpDirectory() + File.separator + getReportName()
                + REPORT_XLS_TYPE;
    }

    private String getJasperPath() {
        return getJasperReportContext().getReportsDirectory() + File.separator + getReportName()
                + REPORT_JASPER_TYPE;
    }

    public void generateReports() {
        JasperPrint jasperPrint = null;
        jasperPrint = getJasperReportContext().getJasperPrint(getJasperPath(),
                getReportParameters(), getReportData());

        generateHtmlReport(jasperPrint);
        generatePdfReport(jasperPrint);
        generateCsvReport(jasperPrint);
        generateXlsReport(jasperPrint);
    }

    private void generateHtmlReport(JasperPrint jasperPrint) {
        HttpSession session = getRequestGlobals().getRequest().getSession(true);
        // Put JasperPrint object in the session for displaying html report images
        session.setAttribute(BaseHttpServlet.DEFAULT_JASPER_PRINT_SESSION_ATTRIBUTE, jasperPrint);
        getJasperReportContext().generateHtmlReport(jasperPrint, getHtmlReportPath());
    }

    private void generatePdfReport(JasperPrint jasperPrint) {
        getJasperReportContext().generatePdfReport(jasperPrint, getPdfReportPath());
    }

    private void generateCsvReport(JasperPrint jasperPrint) {
        getJasperReportContext().generateCsvReport(jasperPrint, getCsvReportPath());
    }

    private void generateXlsReport(JasperPrint jasperPrint) {
        getJasperReportContext().generateXlsReport(jasperPrint, getXlsReportPath());
    }
}
