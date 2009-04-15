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

import java.io.File;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import net.sf.jasperreports.engine.JRException;
import net.sf.jasperreports.engine.JasperCompileManager;
import net.sf.jasperreports.engine.JasperPrint;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class JasperReportContextImplTestIntegration extends IntegrationTestCase {
    private static final String REPORT_NAME = "jasper-report-test";

    private static final String REPORT = TestHelper.getTestDirectory() + "/" + REPORT_NAME;

    private static final String COMPILED_REPORT = REPORT + ".jasper";

    private static final String HTML_REPORT = REPORT + ".html";

    private static final String PDF_REPORT = REPORT + ".pdf";

    private static final String CSV_REPORT = REPORT + ".csv";

    private static final String XLS_REPORT = REPORT + ".xls";

    private final String m_uncompiledReport = TestUtil.getTestSourceDirectory(getClass()) + "/"
            + REPORT_NAME + ".jrxml";

    private JasperReportContext m_jasperReportContext;

    public void setJasperReportContext(JasperReportContext jasperReportContext) {
        m_jasperReportContext = jasperReportContext;
    }

    public void testCompileDesignReport() throws Exception {
        compileDesignReport(m_uncompiledReport, COMPILED_REPORT);
        File jasperReport = new File(COMPILED_REPORT);
        assertTrue(jasperReport.exists());
    }

    public void testGetJasperPrint() throws Exception {
        JasperPrint jasperPrint = getJasperPrint();
        assertNotNull("Jasper print should not be null", jasperPrint);
    }

    public void testGenerateHtmlReport() throws Exception {
        JasperPrint jasperPrint = getJasperPrint();
        m_jasperReportContext.generateHtmlReport(jasperPrint, HTML_REPORT);
        File htmlReport = new File(HTML_REPORT);
        assertTrue(htmlReport.exists());
    }

    public void testGeneratePdfReport() throws Exception {
        JasperPrint jasperPrint = getJasperPrint();
        m_jasperReportContext.generatePdfReport(jasperPrint, PDF_REPORT);
        File pdfReport = new File(PDF_REPORT);
        assertTrue(pdfReport.exists());
    }

    public void testGenerateCsvReport() throws Exception {
        JasperPrint jasperPrint = getJasperPrint();
        m_jasperReportContext.generateCsvReport(jasperPrint, CSV_REPORT);
        File csvReport = new File(CSV_REPORT);
        assertTrue(csvReport.exists());
    }

    public void testGenerateXlsReport() throws Exception {
        JasperPrint jasperPrint = getJasperPrint();
        m_jasperReportContext.generateXlsReport(jasperPrint, XLS_REPORT);
        File xlsReport = new File(XLS_REPORT);
        assertTrue(xlsReport.exists());
    }

    private void compileDesignReport(String designReportPath, String jasperReportPath) {
        try {
            JasperCompileManager.compileReportToFile(designReportPath, jasperReportPath);
        } catch (JRException jrEx) {
            fail("Error compiling jasper-report-test design file ");
        }
    }

    private JasperPrint getJasperPrint() {
        JasperPrint jasperPrint = m_jasperReportContext.getJasperPrint(COMPILED_REPORT,
                getReportParameters(), getData());
        return jasperPrint;
    }

    private Map<String, String> getReportParameters() {
        Map<String, String> mapTableReport = new HashMap<String, String>();
        mapTableReport.put("titleTestReport", "Test Report");

        return mapTableReport;
    }

    private List<MyBean> getData() {
        List<MyBean> list = new ArrayList<MyBean>();

        for (int x = 0; x < 10; x++) {
            MyBean bean = new MyBean();
            bean.setFrom("201");
            bean.setTo("202");
            bean.setStartTime(new Date());
            bean.setStatus("Completed");
            list.add(bean);
        }
        return list;
    }

    public class MyBean {
        private String m_from;

        private String m_to;

        private Date m_startTime;

        private String m_status;

        public String getFrom() {
            return m_from;
        }

        public void setFrom(String from) {
            m_from = from;
        }

        public Date getStartTime() {
            return m_startTime;
        }

        public void setStartTime(Date startTime) {
            m_startTime = startTime;
        }

        public String getStatus() {
            return m_status;
        }

        public void setStatus(String status) {
            m_status = status;
        }

        public String getTo() {
            return m_to;
        }

        public void setTo(String to) {
            m_to = to;
        }
    }
}
