/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.skin;

import java.io.InputStream;
import java.io.OutputStream;

import javax.servlet.ServletConfig;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.io.IOUtils;
import org.apache.tapestry.IAsset;
import org.springframework.context.ApplicationContext;
import org.springframework.web.context.support.WebApplicationContextUtils;

/**
 * Redirect fixed URL to configurable and complex logo url E.g. /images/logo.png to asset for logo
 * passed in SkinControl.getLogoClasspath()
 */
public class SkinServlet extends HttpServlet {
    private SkinControl m_skin;

    public void init(ServletConfig config) throws ServletException {
        ApplicationContext app = WebApplicationContextUtils
                .getRequiredWebApplicationContext(config.getServletContext());
        m_skin = (SkinControl) app.getBean(SkinControl.CONTEXT_BEAN_NAME);
    }

    protected void doGet(HttpServletRequest request, HttpServletResponse response)
        throws javax.servlet.ServletException, java.io.IOException {
        String path = request.getPathInfo().substring(1); // strip '/'
        IAsset asset = m_skin.getAsset(path);
        if (asset != null) {
            InputStream in = asset.getResourceAsStream();
            OutputStream out = response.getOutputStream();
            IOUtils.copy(in, out);
        }
    }

    public void destroy() {
    }
}
