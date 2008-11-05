/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cmcprov;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.Reader;
import java.io.Writer;
import java.util.Map;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.io.IOUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.util.StringUtils;

public class UpdateServlet extends ProvisioningServlet {
    public static final String USERNAME = "username";
    public static final String PASSWORD = "password";
    public static final String MAC_ADDRESS = "mac";
    public static final String USERNAME_PASSWORD_CANNOT_BE_MISSING_ERROR = "Username and password cannot be missing";
    public static final String USERNAME_PASSWORD_ARE_INVALID_ERROR = "Either username or password are invalid";
    private static final Log LOG = LogFactory.getLog(UpdateServlet.class);

    @Override
    public void doGet(HttpServletRequest req, HttpServletResponse resp) throws ServletException, IOException {
        PrintWriter out = resp.getWriter();
        try {
            User user = authenticateRequest(req);
            String profileFilename = getMACAddressFromRequest(req).toLowerCase() + ".ini";
            uploadPhoneProfile(user, profileFilename, out);
        } catch (FailureDataException e) {
            LOG.error("Update error: " + e.getMessage());
            buildFailureResponse(out, e.getMessage());
        }
    }

    private String getMACAddressFromRequest(HttpServletRequest req) {
        Map parameters = req.getParameterMap();
        if (!parameters.containsKey(MAC_ADDRESS)) {
            throw new FailureDataException("MAC address cannot be missing");
        }
        String[] values = (String[]) parameters.get(MAC_ADDRESS);
        String macAddress = StringUtils.delete(values[0], "-");
        return macAddress;
    }

    public void uploadPhoneProfile(User user, String profileFilename, Writer out) {
        String uploadDirectory = getProvisioningContext().getUploadDirectory();
        try {
            attachFile(new File(uploadDirectory, profileFilename), out);
        } catch (IOException e) {
            throw new FailureDataException("Error while uploading configuration file: " + e.getMessage());
        }
    }

    public void attachFile(File file, Writer out) throws IOException {
        Reader in = new FileReader(file);
        IOUtils.copy(in, out);
        IOUtils.closeQuietly(in);
    }

    /**
     * Authenticates a provisioning requests. Returns a User object representing the
     * authenticated user if successful, otherwise throws a Exception.
     */
    protected User authenticateRequest(HttpServletRequest req) {
        Map parameters = req.getParameterMap();
        if (!parameters.containsKey(USERNAME) || !parameters.containsKey(PASSWORD)) {
            throw new FailureDataException(USERNAME_PASSWORD_CANNOT_BE_MISSING_ERROR);
        }
        String[] values = (String[]) parameters.get(USERNAME);
        String username = values[0];
        values = (String[]) parameters.get(PASSWORD);
        String password = values[0];
        User user = getProvisioningContext().getUser(username, password);
        if (user == null) {
            throw new FailureDataException(USERNAME_PASSWORD_ARE_INVALID_ERROR);
        }
        return user;
    }
}
