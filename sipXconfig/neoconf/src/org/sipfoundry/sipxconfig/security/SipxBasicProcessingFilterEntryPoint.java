/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.security;

import java.io.IOException;

import static java.lang.String.format;

import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.ServletResponse;
import javax.servlet.http.HttpServletResponse;

import org.acegisecurity.AuthenticationException;
import org.acegisecurity.ui.AuthenticationEntryPoint;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.annotation.Required;

public class SipxBasicProcessingFilterEntryPoint implements AuthenticationEntryPoint {
    private DomainManager m_domainManager;

    private String m_header;

    private String getAuthenticateHeader() {
        if (null == m_header) {
            m_header = format("Basic realm=\"%s\"", m_domainManager.getAuthorizationRealm());
        }
        return m_header;
    }

    @Override
    public void commence(ServletRequest request, ServletResponse response, AuthenticationException authException)
        throws IOException, ServletException {
        HttpServletResponse httpResponse = (HttpServletResponse) response;
        httpResponse.addHeader("WWW-Authenticate", getAuthenticateHeader());
        httpResponse.sendError(HttpServletResponse.SC_UNAUTHORIZED, authException.getMessage());
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }
}
