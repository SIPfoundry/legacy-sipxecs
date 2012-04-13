/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr.rest;

import java.util.Map;

import org.apache.log4j.Logger;
import org.mortbay.http.HttpContext;
import org.mortbay.http.HttpServer;
import org.mortbay.http.SecurityConstraint;
import org.mortbay.http.SocketListener;
import org.mortbay.jetty.servlet.ServletHandler;
import org.sipfoundry.commons.jetty.SipXSecurityHandler;
import org.sipfoundry.commons.jetty.SocketFactory;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.beans.factory.annotation.Required;

/**
 * Run a Jetty based web server to handle http/https requests for sipXivr
 *
 */
public class WebServer implements BeanFactoryAware {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private ServletHandler m_servletHandler;
    private int m_httpPort;
    private int m_publicHttpPort;
    private BeanFactory m_beanFactory;
    private SipxIvrUserRealm m_userRealm;
    private SipxIvrDigestAuthenticator m_digestAuthenticator;
    private SipXSecurityHandler m_securityHandler;

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

            SocketListener internalListener = SocketFactory.createSocketListener(m_httpPort);
            SocketListener publicListener = SocketFactory.createSocketListener(m_publicHttpPort);
            server.addListener(internalListener);
            server.addListener(publicListener);

            HttpContext httpContext = new HttpContext();
            httpContext.setContextPath("/");
            httpContext.setAuthenticator(m_digestAuthenticator);

            SecurityConstraint digestConstraint = new SecurityConstraint();
            digestConstraint.setName(SecurityConstraint.__DIGEST_AUTH);
            digestConstraint.addRole("IvrRole");
            digestConstraint.setAuthenticate(true);
            httpContext.addSecurityConstraint("/*", digestConstraint);
            httpContext.setRealm(m_userRealm);
            httpContext.addHandler(0, m_securityHandler);
            httpContext.addHandler(1, m_servletHandler);

            server.addContext(httpContext);

            // Start it up.
            LOG.info(String.format("Starting Jetty server on ports *:%d, *:%d", m_httpPort, m_publicHttpPort));
            server.start();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void setServletHandler(ServletHandler handler) {
        m_servletHandler = handler;
    }

    public void setUserRealm(SipxIvrUserRealm realm) {
        m_userRealm = realm;
    }

    @Required
    public void setHttpPort(int httpPort) {
        m_httpPort = httpPort;
    }

    @Required
    public void setPublicHttpPort(int publicHttpPort) {
        m_publicHttpPort = publicHttpPort;
    }

    @Override
    public void setBeanFactory(BeanFactory factory) {
        m_beanFactory = factory;
    }

    @Required
    public void setDigestAuthenticator(SipxIvrDigestAuthenticator digestAuthenticator) {
        m_digestAuthenticator = digestAuthenticator;
    }

    @Required
    public void setSecurityHandler(SipXSecurityHandler securityHandler) {
        m_securityHandler = securityHandler;
    }


}
