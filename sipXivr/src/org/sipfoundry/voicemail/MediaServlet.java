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

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.io.IOUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.commons.util.DomainConfiguration;
import org.sipfoundry.sipxivr.rest.SipxIvrServletHandler;
import org.sipfoundry.voicemail.mailbox.MailboxManager;
import org.sipfoundry.voicemail.mailbox.VmMessage;

public class MediaServlet extends HttpServlet {
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
        ValidUsers validUsers = (ValidUsers) request.getAttribute(SipxIvrServletHandler.VALID_USERS_ATTR);
        MailboxManager mailboxManager = (MailboxManager) request.getAttribute(SipxIvrServletHandler.MAILBOX_MANAGER);
        if (sharedSecret == null) {
            DomainConfiguration config = new DomainConfiguration(System.getProperty("conf.dir") + "/domain-config");
            sharedSecret = config.getSharedSecret();
        }
        String method = request.getMethod().toUpperCase();

        String pathInfo = request.getPathInfo();
        String[] subDirs = pathInfo.split("/");
        if (subDirs.length < 3) {
            response.sendError(404); // path not found
            return;
        }

        // The first element is empty (the leading slash)
        // The second element is the mailbox
        String mailboxString = subDirs[1];
        // The third element is the "context" (either mwi, message)
        String dir = subDirs[2];
        String messageId = subDirs[3];

        User user = validUsers.getUser(mailboxString);
        // only superadmin and mailbox owner can access this service
        // TODO allow all admin user to access it
        boolean trustedSource = request.getAttribute("trustedSource") != null
                && request.getAttribute("trustedSource").equals(sharedSecret);
        if (!trustedSource) {
            String authenticatedUserName = request.getUserPrincipal().getName();
            if (!authenticatedUserName.equals(user.getUserName())) {
                if (!authenticatedUserName.equals("superadmin")) {
                    response.sendError(403); // Send 403 Forbidden
                    return;
                }
            }
        }

        if (user != null) {
            if (method.equals(METHOD_GET)) {
                VmMessage message = mailboxManager.getVmMessage(user.getUserName(), messageId, true);
                response.setHeader("Expires", "0");
                response.setHeader("Cache-Control", "must-revalidate, post-check=0, pre-check=0");
                response.setHeader("Pragma", "public");
                response.setHeader("Content-Disposition", "attachment; filename=\"" + message.getAudioFile().getName() + "\"");

                OutputStream responseOutputStream = null;
                InputStream stream = null;
                try {
                    responseOutputStream = response.getOutputStream();
                    stream = new FileInputStream(message.getAudioFile());
                    IOUtils.copy(stream, responseOutputStream);
                } finally {
                    IOUtils.closeQuietly(stream);
                    IOUtils.closeQuietly(responseOutputStream);
                    message.cleanup();
                }
            } else {
                response.sendError(405);
            }
        }

    }
}
