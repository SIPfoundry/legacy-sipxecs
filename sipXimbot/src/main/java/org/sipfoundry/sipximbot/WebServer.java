/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.sipximbot;

import org.apache.log4j.Logger;
import org.mortbay.http.HttpContext;
import org.mortbay.http.HttpServer;
import org.mortbay.jetty.servlet.ServletHandler;

/**
 * Run a Jetty based web server to handle http/https requests for sipXimbot
 * 
 */
public class WebServer  {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipximbot");
    ServletHandler m_servletHandler;
    ImbotConfiguration m_imbotConfig;
    
    public WebServer(ImbotConfiguration ivrConfig) {
        m_imbotConfig = ivrConfig ;
        m_servletHandler = new ServletHandler();
    }
    
    /**
     * add a servlet for the Web server to use 
     * @param name
     * @param pathSpec
     * @param servletClass must be of type javax.servlet.Servlet
     */
    public void addServlet(String name, String pathSpec, String servletClass) {
        m_servletHandler.addServlet(name, pathSpec, servletClass);
        LOG.info(String.format("Adding Servlet %s on %s", name, pathSpec));
    }
    
    /**
     * Start the Web Server that handles sipXimbot Web requests
     */
    public void start() {
        try {
            // Start up jetty
            HttpServer server = new HttpServer();

            // Bind the port on all interfaces
            // TODO HTTPS support
            int httpsPort = m_imbotConfig.getHttpPort();
            server.addListener(":" + httpsPort);

            HttpContext httpContext = new HttpContext();
            httpContext.setContextPath("/");

            httpContext.addHandler(m_servletHandler);

            server.addContext(httpContext);
            
            // Start it up.
            LOG.info(String.format("Starting Jetty server on *:%d", httpsPort));
            server.start();
        } catch (Exception e) {
            e.printStackTrace(); 
        }
    }

}
