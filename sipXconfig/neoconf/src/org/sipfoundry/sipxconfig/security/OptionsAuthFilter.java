/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.springframework.web.filter.GenericFilterBean;

/**
 * Filter that handles OPTIONS HTTP method
 */
public class OptionsAuthFilter extends GenericFilterBean {
    private static final Log LOG = LogFactory.getLog(OptionsAuthFilter.class);

    @Override
    public void doFilter(ServletRequest paramServletRequest, ServletResponse paramServletResponse,
        FilterChain paramFilterChain) throws IOException, ServletException {
        String httpMethod = ((HttpServletRequest) paramServletRequest).getMethod();
        LOG.debug(String.format("Filtering %s method ", httpMethod));
        if (!"OPTIONS".equals(httpMethod)) {
            // only follow the chain for non-OPTIONS request; these do not need authentication
            paramFilterChain.doFilter(paramServletRequest, paramServletResponse);
        }
    }
}
