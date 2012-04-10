/**
 *
 *
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.voicemail;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.OutputStream;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.io.IOUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.util.DomainConfiguration;
import org.sipfoundry.sipxivr.SipxIvrConfiguration;
import org.sipfoundry.sipxivr.rest.SipxIvrServletHandler;

public class ManagementServlet extends HttpServlet {
    private static final long serialVersionUID = 1L;
    private static final String METHOD_GET = "GET";
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private String sharedSecret = null;

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
        if (pathInfo.endsWith("restore/log")) {
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
}
