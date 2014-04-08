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
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import javax.servlet.FilterChain;
import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.ServletResponse;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletRequestWrapper;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.codec.binary.Base64;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.common.AbstractUser;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.security.web.FilterChainProxy;

public class SipxFilterChainProxy extends FilterChainProxy {
    private static final Log LOG = LogFactory.getLog(SipxFilterChainProxy.class);
    private DomainManager m_domainManager;
    private AdminContext m_adminCtx;

    /**
     * If internal port is used, automatically authenticate using shared secret If other sipxecs
     * components need to call rest services in sipxconfig, no authentication is needed
     */
    @Override
    public void doFilter(ServletRequest request, ServletResponse response, FilterChain chain) throws IOException,
        ServletException {
        ServletRequest requestToFilter = request;
        int port = AdminContext.HTTP_ADDRESS.getCanonicalPort();
        int authPort = AdminContext.HTTP_ADDRESS_AUTH.getCanonicalPort();
        if (request.getLocalPort() == port && request instanceof HttpServletRequest) {
            HttpServletRequest httpRequest = (HttpServletRequest) request;
            requestToFilter = new AuthorizedServletRequest(httpRequest);
            LOG.trace("Internal request port: " + port);
        }
        if (request.getLocalPort() == authPort && request instanceof HttpServletRequest
            && response instanceof HttpServletResponse) {
            HttpServletResponse httpResponse = (HttpServletResponse) response;
            String originUrl = ((HttpServletRequest) request).getHeader("Origin");
            String origin = originUrl;
            String schemeSeparator = "://";
            String portSeparator = ":";
            // it is important to strip the scheme first, because it contains the separator for
            // port
            if (origin != null && origin.indexOf(schemeSeparator) > 0) {
                origin = origin.substring(origin.indexOf(schemeSeparator) + schemeSeparator.length());
            }
            if (origin != null && origin.indexOf(portSeparator) > 0) {
                origin = origin.substring(0, origin.indexOf(portSeparator));
            }
            httpResponse.setHeader("Access-Control-Allow-Origin", originUrl);
            httpResponse.setHeader("Access-Control-Allow-Credentials",
                String.valueOf(getAllowedCorsDomains().contains(origin)));
            httpResponse.setHeader("Access-Control-Allow-Methods", "DELETE, HEAD, GET, PATCH, POST, PUT");
            httpResponse.setHeader("Access-Control-Max-Age", "3600");
            String allowedHeaders;
            if ("OPTIONS".equals(((HttpServletRequest) request).getMethod())) {
                allowedHeaders = "accept, authorization, content-type";
            } else {
                allowedHeaders = "accept, accept-charset, accept-encoding, accept-language, authorization, "
                    + "content-length, content-type, host, origin, proxy-connection, referer, user-agent, "
                    + "x-requested-with";
            }
            httpResponse.setHeader("Access-Control-Allow-Headers", allowedHeaders);
            LOG.trace("Internal request authPort: " + port);
        }
        super.doFilter(requestToFilter, response, chain);
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    @Required
    public void setAdminContext(AdminContext adminContext) {
        m_adminCtx = adminContext;
        LOG.debug("AdminContext: " + m_adminCtx.getClass().getName());
    }

    private List<String> getAllowedCorsDomains() {
        String domains = m_adminCtx.getSettings().getCorsDomains();
        LOG.trace(String.format("Stored domains: [%s]", domains));
        List<String> allowedDomains;
        if (domains != null) {
            allowedDomains = Arrays.asList(domains.split(","));
        } else {
            allowedDomains = Collections.emptyList();
        }
        LOG.trace("Allowed CORS domains: " + allowedDomains);

        return allowedDomains;
    }

    private class AuthorizedServletRequest extends HttpServletRequestWrapper {
        public AuthorizedServletRequest(HttpServletRequest request) {
            super(request);
        }

        @Override
        public String getHeader(String name) {
            if ("Authorization".equals(name)) {
                String authString = String.format("%s:%s", AbstractUser.SUPERADMIN,
                    m_domainManager.getSharedSecret());
                return "Basic " + new String(Base64.encodeBase64(authString.getBytes()));
            }
            return super.getHeader(name);
        }
    }
}
