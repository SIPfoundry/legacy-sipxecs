/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.sipxivr;

import org.apache.log4j.Logger;
import org.mortbay.http.BasicAuthenticator;
import org.mortbay.http.DigestAuthenticator;
import org.mortbay.http.HashUserRealm;
import org.mortbay.http.HttpContext;
import org.mortbay.http.HttpServer;
import org.mortbay.http.SecurityConstraint;
import org.mortbay.http.SslListener;
import org.mortbay.http.UserRealm;
import org.mortbay.http.handler.SecurityHandler;
import org.mortbay.jetty.servlet.ServletHandler;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsersXML;
import org.sipfoundry.commons.util.DomainConfiguration;

/**
 * Run a Jetty based web server to handle http/https requests for sipXivr
 * 
 */
public class WebServer  {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    ServletHandler m_servletHandler;
    IvrConfiguration m_ivrConfig;
    
    public WebServer(IvrConfiguration ivrConfig) {
        m_ivrConfig = ivrConfig ;
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
     * Start the Web Server that handles sipXivr Web requests
     */
    public void start() {
        try {
            // Start up jetty
            HttpServer server = new HttpServer();

            SslListener sslListener = createSslListener();

            HttpContext httpContext = new HttpContext();
            httpContext.setContextPath("/");
            httpContext.setAuthenticator(new SipxIvrDigestAuthenticator());

            SecurityConstraint digestConstraint = new SecurityConstraint();
            digestConstraint.setName(SecurityConstraint.__DIGEST_AUTH);
            digestConstraint.addRole("IvrRole");
            digestConstraint.setAuthenticate(true);
            httpContext.addSecurityConstraint("/*", digestConstraint);

            httpContext.setRealm(createRealm());

            SecurityHandler sh = new SecurityHandler();
            httpContext.addHandler(0, sh);

            httpContext.addHandler(1, m_servletHandler);

            server.addContext(httpContext);
            server.addListener(sslListener);
            
            // Start it up.
            LOG.info(String.format("Starting Jetty server on *:%d", m_ivrConfig.getHttpsPort()));
            server.start();
        } catch (Exception e) {
            e.printStackTrace(); 
        }
    }

    private UserRealm createRealm() throws Exception {
        DomainConfiguration config = new DomainConfiguration(System.getProperty("conf.dir")+"/domain-config");
        return new SipxIvrUserRealm(config.getSipRealm(), config.getSharedSecret());
    }

    private SslListener createSslListener() throws Exception {
        SslListener sslListener = new SslListener();
        int httpsPort = m_ivrConfig.getHttpsPort();
        sslListener.setPort(httpsPort);
        sslListener.setProtocol("SSLv3");
        IvrConfiguration.get();
        String keystore = System.getProperties().getProperty(
                "javax.net.ssl.keyStore");
        LOG.info("keystore = " + keystore);
        sslListener.setKeystore(keystore);
        String algorithm = System.getProperties().getProperty(
                "jetty.x509.algorithm");
        LOG.info("algorithm = " + algorithm);
         sslListener.setAlgorithm(algorithm);
         String password = System.getProperties().getProperty(
                "jetty.ssl.password");
        LOG.info("password = " + password);
        sslListener.setPassword(password);
        String keypassword = System.getProperties().getProperty(
                "jetty.ssl.keypassword");
        LOG.info("keypassword = " + keypassword);
        sslListener.setKeyPassword(keypassword);
        sslListener.setMaxThreads(32);
        sslListener.setMinThreads(4);
        sslListener.setLingerTimeSecs(30000);
        sslListener.setMaxIdleTimeMs(60000);

        return sslListener;
    }

}
