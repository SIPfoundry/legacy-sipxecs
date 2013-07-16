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

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.StringUtils;
import org.springframework.security.web.authentication.SimpleUrlAuthenticationSuccessHandler;
import org.springframework.security.web.savedrequest.HttpSessionRequestCache;
import org.springframework.security.web.savedrequest.SavedRequest;

public class SipxSimpleUrlAuthenticationSuccessHandler extends SimpleUrlAuthenticationSuccessHandler {

    public static final String ORIGINAL_REFERER = "originalReferer";

    @Override
    protected String determineTargetUrl(HttpServletRequest request, HttpServletResponse response) {
        if (isAlwaysUseDefaultTargetUrl()) {
            return getDefaultTargetUrl();
        }

        String targetUrl = obtainFullRequestUrl(request, response);

        // fix for XX-9064 - use http Referer as target url (redirected to)
        if (targetUrl == null) {
            String referer = (String) request.getSession().getAttribute(ORIGINAL_REFERER);

            // if no original http referer saved on session then use current one
            if (referer == null) {
                referer = request.getHeader("Referer");
            }

            // redirect to referer only if mailbox page encoder
            if (StringUtils.contains(referer, "mailbox")) {
                targetUrl = referer;
            }
            request.getSession().removeAttribute(ORIGINAL_REFERER);
        }

        if (targetUrl == null) {
            targetUrl = getDefaultTargetUrl();
        }

        return targetUrl;
    }

    private String obtainFullRequestUrl(HttpServletRequest request, HttpServletResponse response) {
        SavedRequest savedRequest = new HttpSessionRequestCache().getRequest(request, response);
        if (savedRequest == null) {
            return null;
        }
        return savedRequest.getRedirectUrl();
    }
}
