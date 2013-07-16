/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.security;

import static java.lang.String.format;

import java.io.IOException;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.security.core.AuthenticationException;
import org.springframework.security.web.AuthenticationEntryPoint;

public class SipxBasicAuthenticationEntryPoint implements AuthenticationEntryPoint {
    private DomainManager m_domainManager;

    private String m_header;

    private String getAuthenticateHeader() {
        if (null == m_header) {
            m_header = format("Basic realm=\"%s\"", m_domainManager.getAuthorizationRealm());
        }
        return m_header;
    }

    @Override
    public void commence(HttpServletRequest request, HttpServletResponse httpResponse,
            AuthenticationException authException)
        throws IOException, ServletException {
        httpResponse.addHeader("WWW-Authenticate", getAuthenticateHeader());
        httpResponse.sendError(HttpServletResponse.SC_UNAUTHORIZED, authException.getMessage());
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }
}
