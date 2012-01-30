/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.voicemail;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.fileupload.FileItem;
import org.apache.commons.fileupload.disk.DiskFileItemFactory;
import org.apache.commons.fileupload.servlet.ServletFileUpload;
import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.util.DomainConfiguration;
import org.sipfoundry.sipxivr.SipxIvrConfiguration;
import org.sipfoundry.sipxivr.rest.SipxIvrServletHandler;

public class ManagementServlet extends HttpServlet {
    private static final long serialVersionUID = 1L;
    private static final String METHOD_GET = "GET";
    private static final String METHOD_PUT = "PUT";
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private String sharedSecret = null;
    private String backupScript = "sipx-backup";
    private String restoreScript = "sipx-sudo-restore";
    private String archive = "voicemail.tar.gz";

    public void doPut(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        doIt(request, response);
    }

    public void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        doIt(request, response);
    }

    public void doDelete(HttpServletRequest request, HttpServletResponse response) throws ServletException,
            IOException {
        doIt(request, response);
    }

    public void doIt(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        SipxIvrConfiguration ivrConfig = (SipxIvrConfiguration) request
                .getAttribute(SipxIvrServletHandler.IVR_CONFIG_ATTR);

        if (sharedSecret == null) {
            DomainConfiguration config = new DomainConfiguration(System.getProperty("conf.dir") + "/domain-config");
            sharedSecret = config.getSharedSecret();
        }
        String method = request.getMethod().toUpperCase();
        String pathInfo = request.getPathInfo();

        // only trusted source can access this service
        boolean trustedSource = request.getAttribute("trustedSource") != null
                && request.getAttribute("trustedSource").equals(sharedSecret);
        if (!trustedSource) {
            response.sendError(403); // Send 403 Forbidden
            return;
        }
        // make sure backup dir exists
        File backupDir = new File(ivrConfig.getBackupPath());
        if (!backupDir.exists()) {
            backupDir.mkdir();
        }
        if (pathInfo.endsWith("backup")) {
            if (method.equals(METHOD_GET)) {
                OutputStream responseOutputStream = null;
                InputStream stream = null;
                try {
                    if (performBackup(new File(ivrConfig.getBackupPath()), ivrConfig.getBinDirectory())) {
                        response.setHeader("Expires", "0");
                        response.setHeader("Cache-Control", "must-revalidate, post-check=0, pre-check=0");
                        response.setHeader("Pragma", "public");
                        response.setHeader("Content-Disposition", "attachment; filename=\"" + archive + "\"");

                        responseOutputStream = response.getOutputStream();
                        File tempBackupFile = new File(ivrConfig.getBackupPath() + File.separator + archive);
                        stream = new FileInputStream(tempBackupFile);
                        IOUtils.copy(stream, responseOutputStream);
                    } else {
                        response.sendError(500);
                    }
                } catch (Exception ex) {
                    response.sendError(500);
                } finally {
                    IOUtils.closeQuietly(stream);
                    IOUtils.closeQuietly(responseOutputStream);
                }
            } else {
                response.sendError(405);
            }
        } else if (pathInfo.endsWith("restore")) {
            if (method.equals(METHOD_PUT)) {
                File backup = new File(ivrConfig.getBackupPath() + File.separator + archive);
                // delete old backup if exists
                FileUtils.deleteQuietly(backup);
                DiskFileItemFactory fileItemFactory = new DiskFileItemFactory();
                fileItemFactory.setSizeThreshold(1);
                fileItemFactory.setRepository(new File(ivrConfig.getBackupPath()));
                ServletFileUpload uploadHandler = new ServletFileUpload(fileItemFactory);
                try {
                    List<FileItem> items = uploadHandler.parseRequest(request);
                    for (FileItem item : items) {
                        if (!item.isFormField()) {
                            item.write(backup);
                        }
                    }
                    performRestore(backup, ivrConfig.getBinDirectory());
                } catch (Exception e) {
                    LOG.error("Failed to upload backup" + e.getMessage());
                    response.sendError(500);
                }
            } else {
                response.sendError(405);
            }
        } else if (pathInfo.endsWith("restore/log")) {
            if (method.equals(METHOD_GET)) {
                response.setContentType("text/plain");
                OutputStream responseOutputStream = null;
                try {
                    responseOutputStream = response.getOutputStream();
                    File log = new File(ivrConfig.getLogDirectory(), "sipx-restore.log");
                    if (log.exists()) {
                        IOUtils.copy(new FileReader(log), responseOutputStream);
                    }
                } catch (Exception e) {
                    LOG.error("Failed to retrieve restore log" + e.getMessage());
                    response.sendError(500);
                } finally {
                    IOUtils.closeQuietly(responseOutputStream);
                }
            } else {
                response.sendError(405);
            }
        }
    }

    private boolean performBackup(File workingDir, String binDir) throws IOException, InterruptedException {
        ProcessBuilder pb = new ProcessBuilder(binDir + File.separator + backupScript, "-n", "-v");
        Process process = pb.directory(workingDir).start();
        int code = process.waitFor();
        if (code != 0) {
            String errorMsg = String.format("Backup operation failed. Exit code: %d", code);
            LOG.error(errorMsg);
            return false;
        }

        return true;
    }

    private void performRestore(File backup, String binDir) throws IOException, InterruptedException {
        ProcessBuilder pb = new ProcessBuilder(binDir + File.separator + restoreScript, "-v", backup.getPath(),
                "--non-interactive", "--enforce-version");
        pb.directory(backup.getParentFile()).start();
    }
}
