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
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.List;
import java.util.Map;

import javax.servlet.http.HttpSession;

import net.sf.jasperreports.engine.JasperPrint;
import net.sf.jasperreports.j2ee.servlets.BaseHttpServlet;
import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.AbstractPage;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.request.IUploadFile;
import org.apache.tapestry.services.RequestGlobals;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.jasperreports.JasperReportContext;
import org.sipfoundry.sipxconfig.site.common.AssetSelector;

@ComponentClass(allowBody = false, allowInformalParameters = true)
public abstract class ReportComponent extends BaseComponent {
    private static final Log LOG = LogFactory.getLog(ReportComponent.class);

    private static final String REPORT_DESIGN_TYPE = ".jrxml";

    private static final String REPORT_JASPER_TYPE = ".jasper";

    private static final String REPORT_HTML_TYPE = ".html";

    private static final String REPORT_PDF_TYPE = ".pdf";

    private static final String REPORT_CSV_TYPE = ".csv";

    private static final String REPORT_XLS_TYPE = ".xls";

    private static final String BACKUP_TYPE = ".backup";

    private static final String ERROR_BACKUP = "Error during jasper report-design backup";

    private static final String ERROR_RESTORE = "Error during restore jasper report-design from backup";

    @InjectObject(value = "service:tapestry.globals.RequestGlobals")
    public abstract RequestGlobals getRequestGlobals();

    @InjectObject(value = "spring:jasperReportContextImpl")
    public abstract JasperReportContext getJasperReportContext();

    @Parameter(name = "linkName", required = true)
    public abstract String getLinkName();

    @Parameter(name = "reportName", required = true)
    public abstract String getReportName();

    /**
     * One of either 'reportData' OR 'type' must be supplied.
     */
    @Parameter(name = "reportData", required = false)
    public abstract List< ? > getReportData();

    /**
     * One of the following values: 'sipxconfig' or 'sixcdr'.
     */
    @Parameter(name = "type", required = false, defaultValue = "sipxconfig")
    public abstract String getType();

    @Parameter(name = "reportParameters", required = true)
    public abstract Map<String, String> getReportParameters();

    @Parameter(name = "showHtmlLink", required = false, defaultValue = "true")
    public abstract boolean getShowHtmlLink();

    @Parameter(name = "showPdfLink", required = false, defaultValue = "true")
    public abstract boolean getShowPdfLink();

    @Parameter(name = "showCsvLink", required = false, defaultValue = "true")
    public abstract boolean getShowCsvLink();

    @Parameter(name = "showXlsLink", required = false, defaultValue = "true")
    public abstract boolean getShowXlsLink();

    public abstract IUploadFile getUploadDesignReportFile();

    public boolean isReportsGenerated() {
        File htmlReport = new File(getHtmlReportPath());
        File pdfReport = new File(getPdfReportPath());
        File csvReport = new File(getCsvReportPath());
        File xlsReport = new File(getXlsReportPath());
        if (htmlReport.exists() && pdfReport.exists() && csvReport.exists() && xlsReport.exists()) {
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
        if  (getReportData() != null) {
            jasperPrint = getJasperReportContext().getJasperPrint(getJasperPath(),
                    getReportParameters(), getReportData());
        } else {
            jasperPrint = getJasperReportContext().getJasperPrint(getJasperPath(),
                    getReportParameters(), getType());
        }
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

    public void newDesignReportFile() {
        AbstractPage page = (AbstractPage) getPage();
        IValidationDelegate validator = TapestryUtils.getValidator(page);
        try {
            // Prevent loosing design report
            backupDesignReport();
            // Upload new design report
            upload(getUploadDesignReportFile());
            // Compile new design report
            getJasperReportContext().compileDesignReport(getDesignReportPath(), getJasperPath());
            // Generate reports from compiled design report
            generateReports();
        } catch (ValidatorException e) {
            validator.record(e);
        } catch (UserException ex) {
            // Restore design report from the previous backup
            restoreDesignReportFromBackup();
            validator.record(new ValidatorException(getMessages().getMessage(
                    "error.compilingDesignReport")));
        }
    }

    private void backupDesignReport() {
        try {
            FileUtils.copyFile(new File(getDesignReportPath()), new File(getDesignReportPath()
                    + BACKUP_TYPE));
        } catch (IOException ioEx) {
            LOG.error(ERROR_BACKUP, ioEx);
        }
    }

    private void restoreDesignReportFromBackup() {
        try {
            FileUtils.copyFile(new File(getDesignReportPath() + BACKUP_TYPE), new File(
                    getDesignReportPath()));
            getJasperReportContext().compileDesignReport(getDesignReportPath(), getJasperPath());
        } catch (IOException ioEx) {
            LOG.error(ERROR_RESTORE, ioEx);
        }
    }

    private void upload(IUploadFile uploadFile) throws ValidatorException {
        if (uploadFile == null) {
            return;
        }
        String fileName = AssetSelector.getSystemIndependentFileName(uploadFile.getFilePath());
        if (!fileName.endsWith(REPORT_DESIGN_TYPE)) {
            String error = getMessages().getMessage("message.wrongFileToUpload");
            throw new ValidatorException(error);
        }

        OutputStream os = null;
        try {
            File file = new File(getDesignReportPath());
            os = new FileOutputStream(file);
            IOUtils.copy(uploadFile.getStream(), os);
        } catch (IOException ex) {
            String error = getMessages().getMessage("message.failed.uploadReportDesignFile");
            throw new ValidatorException(error);
        } finally {
            IOUtils.closeQuietly(os);
        }
    }
}
