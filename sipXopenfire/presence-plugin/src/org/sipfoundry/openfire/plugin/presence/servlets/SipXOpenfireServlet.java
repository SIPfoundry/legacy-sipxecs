/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.plugin.presence.servlets;

import java.io.IOException;

import javax.servlet.ServletConfig;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.server.PropertyHandlerMapping;
import org.apache.xmlrpc.server.XmlRpcServerConfigImpl;
import org.apache.xmlrpc.webserver.XmlRpcServletServer;
import org.jivesoftware.admin.AuthCheckFilter;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class SipXOpenfireServlet extends HttpServlet {
    /**
     *
     */
    private static final long serialVersionUID = 1L;

    private XmlRpcServletServer server;

    private static final Logger log = LoggerFactory.getLogger(SipXOpenfireServlet.class);

    private String path;

    public void init(ServletConfig servletConfig, String serverName, String serviceName, Class< ? > provider)
            throws ServletException {

        super.init(servletConfig);
        log.info(String.format("initializing Servlet for service name %s and provider %s", serviceName,
                provider.getCanonicalName()));

        // Exclude this servlet from requiring the user to login
        this.path = "sipx-openfire-presence/" + serviceName;
        AuthCheckFilter.addExclude(path);

        PropertyHandlerMapping handlerMapping = new PropertyHandlerMapping();

        try {
            handlerMapping.setAuthenticationHandler(new BasicXmlRpcAuthenticationHandler());
            handlerMapping.addHandler(serverName, provider);
        } catch (XmlRpcException e) {
            throw new ServletException("XmlRpcInitialization failed");
        }

        server = new XmlRpcServletServer();

        XmlRpcServerConfigImpl serverConfig = new XmlRpcServerConfigImpl();
        serverConfig.setKeepAliveEnabled(true);
        serverConfig.setEnabledForExceptions(true);
        serverConfig.setEnabledForExtensions(true);
        server.setMaxThreads(4);

        server.setConfig(serverConfig);
        server.setHandlerMapping(handlerMapping);
    }

    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException,
            IOException {
        server.execute(request, response);
    }

    @Override
    public void destroy() {
        super.destroy();

        // Release the excluded URL
        AuthCheckFilter.removeExclude(this.path);
    }

}
