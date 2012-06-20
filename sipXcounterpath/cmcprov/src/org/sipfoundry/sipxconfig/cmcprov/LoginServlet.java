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

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.net.URLDecoder;
import java.util.Hashtable;
import java.util.Map;
import java.util.Properties;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.phone.Phone;

public class LoginServlet extends ProvisioningServlet {
    public static final String PARAMETER_ENCODING = "UTF-8";
    public static final String USERNAME_PASSWORD_CANNOT_BE_MISSING_ERROR = "Username and password cannot be missing";
    public static final String USERNAME_PASSWORD_ARE_INVALID_ERROR = "Either username or password are invalid";
    public static final String INVALID_CREDIDENTIALS = "Your credentials are not recognized";
    private static final Log LOG = LogFactory.getLog(LoginServlet.class);
    private static final String WWW_DIR_PROPERTY = "www.dir";
    private static final String PHONE_DIR_PROPERTY = "sipxconfig.phone.dir";
    private static final String CONF_DIR_PROPERTY = "conf.dir";
    private static final String CONF_RESOURCE = "/counterpath/cmcprov.properties";
    private static final String CONTACTS_LIST_FILE_SUBFIX = "-directory.xml";
    private static final String WEBDAV_DIR = "/webdav/";
    private static final String TFTP_RELATIVE_PATH = "/profile/tftproot/";
    private static final String DOT = ".";

    @Override
    protected void doPost(HttpServletRequest req, HttpServletResponse resp) throws javax.servlet.ServletException,
            java.io.IOException {
        PrintWriter out = resp.getWriter();
        try {
            User user = authenticateRequest(req);
            Phone phone = getProvisioningContext().getPhoneForUser(user);
            if (phone == null) {
                throw new FailureDataException(INVALID_CREDIDENTIALS);
            }
            Profile[] profileFilenames = phone.getProfileTypes();

            FileInputStream fin = new FileInputStream(getProvisioningContext().getConfDir() + CONF_RESOURCE);
            Properties properties = new Properties();

            try {
                properties.load(fin);
            } catch (IOException ex) {
                LOG.error("loading error:  " + CONF_RESOURCE + ex.getMessage());
            }

            fin.close();

            updateContactList(user, phone, properties.getProperty(WWW_DIR_PROPERTY), properties
                    .getProperty(PHONE_DIR_PROPERTY));
            uploadPhoneProfile(profileFilenames[0].getName(), out);
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

    private void updateContactList(User user, Phone phone, String wwwdir, String phonedir) {
        String domainName = this.getProvisioningContext().getDomainName();
        String phoneBookName = phone.getSerialNumber() + CONTACTS_LIST_FILE_SUBFIX;
        String contactListFilePath = wwwdir + WEBDAV_DIR + user.getUserName() + DOT + domainName + DOT
                + phoneBookName;
        String phoneBookFilePath = phonedir + TFTP_RELATIVE_PATH + phoneBookName;

        ContactSynchronizer synchronizer = ContactSynchronizer.getInstance(phoneBookFilePath, contactListFilePath);
        synchronizer.synChronize();
    }

}
