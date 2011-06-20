/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cmcprov;

import java.io.IOException;
import java.io.PrintWriter;
import java.util.Hashtable;
import java.util.Map;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.User;

public class UpdateServlet extends ProvisioningServlet {
    public static final String USERNAME_PASSWORD_CANNOT_BE_MISSING_ERROR = "Username and password cannot be missing";
    public static final String USERNAME_PASSWORD_ARE_INVALID_ERROR = "Either username or password are invalid";
    private static final Log LOG = LogFactory.getLog(UpdateServlet.class);

    @Override
    public void doGet(HttpServletRequest req, HttpServletResponse resp) throws ServletException,
            IOException {
        PrintWriter out = resp.getWriter();
        try {
            authenticateRequest(req);

            Map<String, String> settings = new Hashtable<String, String>();
            String queryString = "?username=$username$&password=$password$";
            String configServerURL = req.getRequestURL().substring(0,
                    req.getRequestURL().indexOf(req.getServletPath()))
                    + UPDATE_SERVLET + queryString;
            settings.put("system:auto_update:config_server_url", configServerURL);
            settings.put("feature:auto_update:config_server_url", configServerURL);

            buildSuccessResponse(out, settings);
        } catch (FailureDataException e) {
            LOG.error("Update error: " + e.getMessage());
            buildFailureResponse(out, e.getMessage());
        }
    }

    /**
     * Authenticates a provisioning requests. Returns a User object representing the authenticated
     * user if successful, otherwise throws a Exception.
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
