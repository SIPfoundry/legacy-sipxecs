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

import java.io.BufferedReader;
import java.io.PrintWriter;
import java.net.URLDecoder;
import java.util.Hashtable;
import java.util.Map;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.User;

public class LoginServlet extends ProvisioningServlet {
    public static final String UPDATE_SERVLET = "/update";
    public static final String USERNAME = "username";
    public static final String PASSWORD = "password";
    public static final String PARAMETER_ENCODING = "UTF-8";
    public static final String USERNAME_PASSWORD_CANNOT_BE_MISSING_ERROR = "Username and password cannot be missing";
    public static final String USERNAME_PASSWORD_ARE_INVALID_ERROR = "Either username or password are invalid";
    private static final Log LOG = LogFactory.getLog(LoginServlet.class);

    protected void doPost(HttpServletRequest req, HttpServletResponse resp) throws javax.servlet.ServletException,
            java.io.IOException {
        PrintWriter out = resp.getWriter();
        try {
            Map<String, String> settings = new Hashtable<String, String>();
            String queryString = "?username=$username$&password=$password$&mac=$MAC$";
            String configServerURL = req.getRequestURL().substring(0,
                    req.getRequestURL().indexOf(req.getServletPath()))
                    + UPDATE_SERVLET + queryString;
            settings.put("system:auto_update:config_server_url", configServerURL);
            settings.put("feature:auto_update:config_server_url", configServerURL);
            buildSuccessResponse(out, settings);
        } catch (FailureDataException e) {
            LOG.error("Login error: " + e.getMessage());
            buildFailureResponse(out, e.getMessage());
        }
    }

    protected User authenticateRequest(HttpServletRequest req) {
        /*
         * Authenticates a login requests. Returns a User object representing the authenticated
         * user if succesful, otherwise throws a Exception.
         */
        User user;
        String username;
        String password;
        Map<String, String> parameters;
        try {
            parameters = getParameterMapFromBody(req);
        } catch (java.io.IOException error) {
            throw new FailureDataException("Cannot extract parameters");
        }
        if (!parameters.containsKey(USERNAME) || !parameters.containsKey(PASSWORD)) {
            throw new FailureDataException(USERNAME_PASSWORD_CANNOT_BE_MISSING_ERROR);
        }
        username = parameters.get(USERNAME);
        password = parameters.get(PASSWORD);
        user = getProvisioningContext().getUser(username, password);
        if (user == null) {
            throw new FailureDataException(USERNAME_PASSWORD_ARE_INVALID_ERROR);
        }
        return user;
    }

    protected Map<String, String> getParameterMapFromBody(HttpServletRequest req) throws java.io.IOException {
        Hashtable<String, String> parameters = new Hashtable<String, String>();
        BufferedReader br = req.getReader();
        String line = br.readLine();
        if (line == null) {
            return parameters;
        }
        String[] pairs = line.split("\\&");
        for (int i = 0; i < pairs.length; i++) {
            String[] fields = pairs[i].split("=");
            if (fields.length == 2) {
                String name = URLDecoder.decode(fields[0], PARAMETER_ENCODING);
                String value = URLDecoder.decode(fields[1], PARAMETER_ENCODING);
                parameters.put(name.toLowerCase(), value);
            }
        }
        return parameters;
    }
}
