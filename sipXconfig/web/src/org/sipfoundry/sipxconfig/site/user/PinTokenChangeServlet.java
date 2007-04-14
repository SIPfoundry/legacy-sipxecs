/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import java.util.StringTokenizer;

import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.springframework.context.ApplicationContext;
import org.springframework.web.context.support.WebApplicationContextUtils;

/**
 * Single servlet to change pin. This will get replaced with full-blown SOAP API or equivalent
 * eventually.
 * 
 * Use PinTokenChangeServlet to test. If you want to test from commandline, use
 * 
 * <pre>
 *        curl --data &quot;testuser;oldpintoken;newpintoken&quot; http://localhost:9999/sipxconfig/api/change-pintoken
 * </pre>
 */
public class PinTokenChangeServlet extends HttpServlet {

    private static final int EXPECTED_TOKEN_COUNT = 3;

    protected void doPost(HttpServletRequest req, HttpServletResponse resp)
        throws javax.servlet.ServletException, java.io.IOException {

        String body = IOUtils.toString(req.getInputStream(), "UTF-8");
        try {
            ApplicationContext app = WebApplicationContextUtils
                    .getRequiredWebApplicationContext(getServletContext());
            CoreContext core = (CoreContext) app.getBean(CoreContext.CONTEXT_BEAN_NAME);

            changePin(core, body);
            resp.setContentType("application/text");
        } catch (ChangePinException e) {
            resp.sendError(HttpServletResponse.SC_BAD_REQUEST, e.getMessage());
        }
    }

    static class ChangePinException extends RuntimeException {
        ChangePinException(String msg) {
            super(msg);
        }
    }

    void changePin(CoreContext core, String body) {
        StringTokenizer toks = new StringTokenizer(body, ";");
        if (toks.countTokens() != EXPECTED_TOKEN_COUNT) {
            throw new ChangePinException(
                    "Illformated request body, expected: userid;oldpintoken;newpintoken ");
        }

        String userid = toks.nextToken();
        String oldpin = toks.nextToken();
        String newpin = toks.nextToken();
        User user = core.loadUserByUserName(userid);
        String errMsg = "User and/or pintoken is incorrect for user " + userid;
        if (user == null) {
            throw new ChangePinException(errMsg);
        }

        if (!oldpin.equals(user.getPintoken())) {
            throw new ChangePinException(errMsg);
        }

        if (!user.hasPermission(PermissionName.TUI_CHANGE_PIN)) {
            throw new ChangePinException(
                    "User does not have permission to change PIN value from this API");
        }

        user.setPintoken(newpin);
        core.saveUser(user);
    }
}
