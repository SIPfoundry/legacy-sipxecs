/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr.rest;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.apache.log4j.Logger;
import org.mortbay.http.HttpContext;
import org.mortbay.http.HttpException;
import org.mortbay.http.HttpRequest;
import org.mortbay.http.HttpResponse;
import org.mortbay.http.HttpServer;
import org.mortbay.http.SecurityConstraint;
import org.mortbay.http.SslListener;
import org.mortbay.http.handler.SecurityHandler;
import org.mortbay.jetty.servlet.ServletHandler;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

/**
 * Run a Jetty based web server to handle http/https requests for sipXivr
 *
 */
public class WebServer implements BeanFactoryAware {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private ServletHandler m_servletHandler;
    private String m_configAddress;
    private int m_httpsPort;
    private BeanFactory m_beanFactory;
    private SipxIvrUserRealm m_userRealm;
    private String m_secret;
    public static final String TRUSTED_SOURCE = "127.0.0.1";

    public void init() {
        Map<String, RestApiBean> beans = ((ListableBeanFactory) m_beanFactory).getBeansOfType(RestApiBean.class);
        for (RestApiBean bean : beans.values()) {
            addServlet(bean.getName(), bean.getPathSpec(), bean.getServletClass());
        }
        start();
    }

    /**
     * add a servlet for the Web server to use
     *
     * @param name
     * @param pathSpec
     * @param servletClass must be of type javax.servlet.Servlet
     */
    private void addServlet(String name, String pathSpec, String servletClass) {
        m_servletHandler.addServlet(name, pathSpec, servletClass);
        LOG.info(String.format("Adding Servlet %s on %s", name, pathSpec));
    }

    /**
     * Start the Web Server that handles sipXivr Web requests
     */
    private void start() {
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

            httpContext.setRealm(m_userRealm);

            CustomSecurityHandler sh = new CustomSecurityHandler();

            sh.addTrustedSource("localhost");
            sh.addTrustedSource(m_configAddress);
            sh.addSharedSecret(m_secret);
            sh.addTrustedSource(TRUSTED_SOURCE);
            httpContext.addHandler(0, sh);

            httpContext.addHandler(1, m_servletHandler);

            server.addContext(httpContext);
            server.addListener(sslListener);

            // Start it up.
            LOG.info(String.format("Starting Jetty server on *:%d", m_httpsPort));
            server.start();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private SslListener createSslListener() throws Exception {
        SslListener sslListener = new SslListener();
        sslListener.setPort(m_httpsPort);
        sslListener.setProtocol("SSLv3");
        String keystore = System.getProperties().getProperty("javax.net.ssl.keyStore");
        LOG.info("keystore = " + keystore);
        sslListener.setKeystore(keystore);
        String algorithm = System.getProperties().getProperty("jetty.x509.algorithm");
        LOG.info("algorithm = " + algorithm);
        sslListener.setAlgorithm(algorithm);
        String password = System.getProperties().getProperty("jetty.ssl.password");
        LOG.info("password = " + password);
        sslListener.setPassword(password);
        String keypassword = System.getProperties().getProperty("jetty.ssl.keypassword");
        LOG.info("keypassword = " + keypassword);
        sslListener.setKeyPassword(keypassword);
        sslListener.setMaxThreads(32);
        sslListener.setMinThreads(4);
        sslListener.setLingerTimeSecs(30000);
        sslListener.setMaxIdleTimeMs(60000);

        return sslListener;
    }

    private class CustomSecurityHandler extends SecurityHandler {
        private List<String> _hosts = new ArrayList<String>();
        private String _secret = null;

        public void addTrustedSource(String ipSource) {
            _hosts.add(ipSource);
        }

        public void addSharedSecret(String secret) {
            _secret = secret;
        }

        public void handle(String pathInContext, String pathParams, HttpRequest request, HttpResponse response)
                throws HttpException, IOException {
            if (!_hosts.contains(request.getRemoteAddr())) {
                getHttpContext().checkSecurityConstraints(pathInContext, request, response);
            } else {
                request.setAttribute("trustedSource", _secret);
            }
        }
    }

    public void setConfigAddress(String address) {
        m_configAddress = address;
    }

    public void setServletHandler(ServletHandler handler) {
        m_servletHandler = handler;
    }

    public void setUserRealm(SipxIvrUserRealm realm) {
        m_userRealm = realm;
    }

    public void setHttpsPort(int port) {
        m_httpsPort = port;
    }

    public void setSecret(String secret) {
        m_secret = secret;
    }

    @Override
    public void setBeanFactory(BeanFactory factory) {
        m_beanFactory = factory;
    }

}
