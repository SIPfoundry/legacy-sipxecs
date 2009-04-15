/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.rest;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import com.noelios.restlet.ext.servlet.ServletConverter;

import org.restlet.Restlet;
import org.restlet.Router;
import org.springframework.context.ApplicationContext;

import static org.springframework.web.context.support.WebApplicationContextUtils.getRequiredWebApplicationContext;

/**
 * Style II of RESTlet integration where you keep Spring as ApplicationContext and pull in
 * restlets
 */
public class RestSpringAdapterServlet extends HttpServlet {
    private ServletConverter m_converter;
    private final Map<String, Restlet> m_resourceMappings = new HashMap<String, Restlet>();

    public void init(Router router) {
        for (String key : m_resourceMappings.keySet()) {
            router.attach(key, m_resourceMappings.get(key));
        }
    }

    @Override
    public void init() throws ServletException {
        super.init();
        ApplicationContext app = getRequiredWebApplicationContext(getServletContext());
        RestManager manager = (RestManager) app.getBean(RestManager.CONTEXT_BEAN_NAME);
        m_converter = new ServletConverter(getServletContext());
        Router router = new Router(m_converter.getContext());
        manager.init(router);
        m_converter.setTarget(router);
    }

    @Override
    protected void service(HttpServletRequest req, HttpServletResponse res) throws ServletException, IOException {
        m_converter.service(req, res);
    }
}
