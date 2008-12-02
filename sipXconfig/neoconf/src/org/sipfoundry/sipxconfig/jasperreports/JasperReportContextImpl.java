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
import java.sql.Connection;
import java.sql.SQLException;
import java.util.List;
import java.util.Map;

import net.sf.jasperreports.engine.JRException;
import net.sf.jasperreports.engine.JRExporter;
import net.sf.jasperreports.engine.JRExporterParameter;
import net.sf.jasperreports.engine.JasperCompileManager;
import net.sf.jasperreports.engine.JasperFillManager;
import net.sf.jasperreports.engine.JasperPrint;
import net.sf.jasperreports.engine.data.JRBeanCollectionDataSource;
import net.sf.jasperreports.engine.export.JRCsvExporter;
import net.sf.jasperreports.engine.export.JRHtmlExporter;
import net.sf.jasperreports.engine.export.JRHtmlExporterParameter;
import net.sf.jasperreports.engine.export.JRPdfExporter;
import net.sf.jasperreports.engine.export.JRXlsExporter;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.Session;
import org.sipfoundry.sipxconfig.cdr.CdrManagerImpl;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class JasperReportContextImpl extends HibernateDaoSupport implements JasperReportContext {
    private static final Log LOG = LogFactory.getLog(JasperReportContextImpl.class);

    private static final String REPORT_DESIGN_TYPE = ".jrxml";

    private static final String REPORT_JASPER_TYPE = ".jasper";

    private static final String ERROR_COMPILING = "Error compiling jasper report-design file ";

    private static final String ERROR_FILLING = "Error filling compiled report design ";

    private static final String ERROR_GENERATING = "Error generating report from jasper report ";

    private static final String EXCEPTION_FILLING = "error.fillingDesignReport";

    private String m_reportsDirectory;

    private String m_tmpDirectory;

    private CdrManagerImpl m_cdrManager;

    public void init() {
        File file = new File(m_reportsDirectory);
        if (file.isDirectory()) {
            for (String fileName : file.list()) {
                if (fileName.endsWith(REPORT_DESIGN_TYPE)) {
                    try {
                        String prefix = StringUtils.substringBeforeLast(fileName, ".");
                        String path = m_reportsDirectory + File.separator;
                        JasperCompileManager.compileReportToFile(path + fileName, path + prefix
                                + REPORT_JASPER_TYPE);
                    } catch (JRException jrEx) {
                        LOG.error(ERROR_COMPILING + fileName, jrEx);
                    }
                }
            }
        }
    }

    public void compileDesignReport(String designReportPath, String jasperReportPath) {
        try {
            JasperCompileManager.compileReportToFile(designReportPath, jasperReportPath);
        } catch (JRException jrEx) {
            LOG.error(ERROR_COMPILING + designReportPath, jrEx);
            throw new UserException("error.compilingDesignReport");
        }
    }

    public JasperPrint getJasperPrint(String jasperPath, Map parameters, List< ? > dataSource) {
        JasperPrint jasperPrint = null;
        try {
            jasperPrint = JasperFillManager.fillReport(jasperPath, parameters,
                    new JRBeanCollectionDataSource(dataSource));
        } catch (JRException jrEx) {
            jasperPrint = new JasperPrint();
            LOG.error(ERROR_FILLING + jasperPath, jrEx);
            throw new UserException(false, EXCEPTION_FILLING);
        }

        return jasperPrint;
    }

    public JasperPrint getJasperPrint(String jasperPath, Map parameters, String type) {
        Session session = null;
        Connection conn = null;
        try {
            if (type.equals("sipxconfig")) {
                session = getHibernateTemplate().getSessionFactory().openSession();
                conn = session.connection();
            } else if (type.equals("sipxcdr")) {
                conn = getCdrManager().getDataSource().getConnection();
            }
        } catch (SQLException ex) {
            LOG.error("Cannot obtain connection");
        }

        JasperPrint jasperPrint = null;
        try {
            jasperPrint = JasperFillManager.fillReport(jasperPath, parameters, conn);
        } catch (JRException jrEx) {
            jasperPrint = new JasperPrint();
            LOG.error(ERROR_FILLING + jasperPath, jrEx);
            throw new UserException(false, EXCEPTION_FILLING);
        } finally {
            if (session != null) {
                session.close();
            }
            if (conn != null) {
                try {
                    conn.close();
                } catch (SQLException ex) {
                    LOG.error("Cannot close connection");
                }
            }
        }

        return jasperPrint;
    }

    public void generateHtmlReport(JasperPrint jasperPrint, String htmlFile) {
        JRHtmlExporter exporter = new JRHtmlExporter();
        try {
            exporter.setParameter(JRExporterParameter.JASPER_PRINT, jasperPrint);
            exporter.setParameter(JRExporterParameter.OUTPUT_FILE_NAME, htmlFile);
            exporter.setParameter(JRHtmlExporterParameter.IMAGES_URI, "image?image=");
            exporter.exportReport();
        } catch (JRException jrEx) {
            LOG.error(ERROR_GENERATING + jasperPrint.getName(), jrEx);
        }
    }

    public void generatePdfReport(JasperPrint jasperPrint, String pdfFile) {
        render(new JRPdfExporter(), jasperPrint, pdfFile);
    }

    public void generateCsvReport(JasperPrint jasperPrint, String csvFile) {
        render(new JRCsvExporter(), jasperPrint, csvFile);
    }

    public void generateXlsReport(JasperPrint jasperPrint, String xlsFile) {
        render(new JRXlsExporter(), jasperPrint, xlsFile);
    }

    private static void render(JRExporter exporter, JasperPrint print, String destFileName) {
        try {
            exporter.setParameter(JRExporterParameter.JASPER_PRINT, print);
            exporter.setParameter(JRExporterParameter.OUTPUT_FILE_NAME, destFileName);
            exporter.exportReport();
        } catch (JRException jrEx) {
            LOG.error(ERROR_GENERATING + print.getName(), jrEx);
        }
    }

    @Required
    public void setReportsDirectory(String reportsDirectory) {
        m_reportsDirectory = reportsDirectory;
    }

    public String getReportsDirectory() {
        return m_reportsDirectory;
    }

    public void setTmpDirectory(String tmpDirectory) {
        m_tmpDirectory = tmpDirectory;
    }

    public String getTmpDirectory() {
        return m_tmpDirectory;
    }

    public CdrManagerImpl getCdrManager() {
        return m_cdrManager;
    }

    public void setCdrManager(CdrManagerImpl cdrManager) {
        m_cdrManager = cdrManager;
    }
}
