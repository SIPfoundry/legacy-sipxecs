/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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

import java.io.IOException;

import javax.servlet.FilterChain;
import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.ServletResponse;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletRequestWrapper;

import org.acegisecurity.util.FilterChainProxy;
import org.apache.commons.codec.binary.Base64;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.common.AbstractUser;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.annotation.Required;

public class SipxFilterChainProxy extends FilterChainProxy {
    private static final Log LOG = LogFactory.getLog(SipxFilterChainProxy.class);
    private DomainManager m_domainManager;
    /**
     * If internal port is used, automatically authenticate using shared secret
     * If other sipxecs components need to call rest services in sipxconfig, no authentication
     * is needed
     */
    @Override
    public void doFilter(ServletRequest request, ServletResponse response, FilterChain chain)
        throws IOException, ServletException {
        ServletRequest requestToFilter = request;
        int port = AdminContext.HTTP_ADDRESS.getCanonicalPort();
        if (request.getLocalPort() == port && request instanceof HttpServletRequest) {
            HttpServletRequest httpRequest = (HttpServletRequest) request;
            requestToFilter = new AuthorizedServletRequest(httpRequest);
            LOG.debug("Internal request port: " + port);
        }
        super.doFilter(requestToFilter, response, chain);
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    private class AuthorizedServletRequest extends HttpServletRequestWrapper {
        public AuthorizedServletRequest(HttpServletRequest request) {
            super(request);
        }

        public String getHeader(String name) {
            if (name.equals("Authorization")) {
                String authString = AbstractUser.SUPERADMIN + ":" + m_domainManager.getSharedSecret();
                return "Basic " + new String(Base64.encodeBase64(authString.getBytes()));
            } else {
                return super.getHeader(name);
            }
        }
    }
}
