/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.security;

import javax.servlet.http.HttpServletRequest;

import org.acegisecurity.ui.webapp.AuthenticationProcessingFilter;

public class SipxAuthenticationProcessingFilter extends AuthenticationProcessingFilter {

    public static final String ORIGINAL_REFERER = "originalReferer";

    protected String determineTargetUrl(HttpServletRequest request) {
        if (isAlwaysUseDefaultTargetUrl()) {
            return getDefaultTargetUrl();
        }

        String targetUrl = obtainFullRequestUrl(request);

        // fix for XX-9064 - use http Referer as target url (redirected to)
        if (targetUrl == null) {
            String referer = (String) request.getSession().getAttribute(ORIGINAL_REFERER);

            // if no original http referer saved on session that use current one
            if (referer == null) {
                referer = request.getHeader("Referer");
            }

            targetUrl = referer;
            request.getSession().removeAttribute(ORIGINAL_REFERER);
        }

        if (targetUrl == null) {
            targetUrl = getDefaultTargetUrl();
        }

        return targetUrl;
    }
}
