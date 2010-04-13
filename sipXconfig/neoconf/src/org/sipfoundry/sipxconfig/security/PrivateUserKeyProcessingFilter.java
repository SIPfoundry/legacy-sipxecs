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
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.servlet.Filter;
import javax.servlet.FilterChain;
import javax.servlet.FilterConfig;
import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.ServletResponse;
import javax.servlet.http.HttpServletRequest;

import org.acegisecurity.Authentication;
import org.acegisecurity.context.SecurityContextHolder;
import org.acegisecurity.providers.UsernamePasswordAuthenticationToken;
import org.acegisecurity.userdetails.UserDetails;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.login.PrivateUserKeyManager;
import org.springframework.beans.factory.annotation.Required;

public class PrivateUserKeyProcessingFilter implements Filter {
    private static final Log LOG = LogFactory.getLog(PrivateUserKeyProcessingFilter.class);

    private PrivateUserKeyManager m_privateUserKeyManager;

    /**
     * Does nothing - we reply on IoC lifecycle services instead.
     */

    public void init(FilterConfig ignored) throws ServletException {
    }

    /**
     * Does nothing - we reply on IoC lifecycle services instead.
     */
    public void destroy() {
    }

    public void doFilter(ServletRequest request, ServletResponse response, FilterChain chain) throws IOException,
            ServletException {

        if (!(request instanceof HttpServletRequest)) {
            throw new ServletException("Can only process HttpServletRequest");
        }

        HttpServletRequest httpRequest = (HttpServletRequest) request;

        if (LOG.isDebugEnabled()) {
            LOG.debug("Request is to process authentication");
        }

        if (SecurityContextHolder.getContext().getAuthentication() == null) {
            Authentication token = attemptAuthentication(httpRequest);
            SecurityContextHolder.getContext().setAuthentication(token);
        }

        chain.doFilter(request, response);
    }

    public Authentication attemptAuthentication(HttpServletRequest request) {
        String requestURI = request.getRequestURI();
        Pattern pattern = Pattern.compile("/private/([^/]{32})");
        Matcher matcher = pattern.matcher(requestURI);
        if (!matcher.find()) {
            LOG.warn("No private key found in the URL.");
            return null;
        }
        String privateKey = matcher.group(1);
        User user = m_privateUserKeyManager.getUserFromPrivateKey(privateKey);
        if (user == null) {
            LOG.warn("Bad private key found in the URL");
            return null;
        }

        UserDetails details = new UserDetailsImpl(user, user.getUserName(), UserRole.User.toAuth());
        UsernamePasswordAuthenticationToken result = new UsernamePasswordAuthenticationToken(details, user
                .getPintoken(), details.getAuthorities());
        result.setDetails(details);
        return result;
    }

    @Required
    public void setPrivateUserKeyManager(PrivateUserKeyManager privateUserKeyManager) {
        m_privateUserKeyManager = privateUserKeyManager;
    }
}
