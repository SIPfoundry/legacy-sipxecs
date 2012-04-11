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
package org.sipfoundry.commons.security;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.mortbay.http.HttpException;
import org.mortbay.http.HttpRequest;
import org.mortbay.http.HttpResponse;
import org.mortbay.http.handler.SecurityHandler;

public class CustomSecurityHandler extends SecurityHandler {
    public static final String TRUSTED_SOURCE_KEY = "trustedSource";
    private List<String> _hosts = new ArrayList<String>();
    private String _secret = null;
    public void addTrustedSource(String ipSource) {
        _hosts.add(ipSource);
    }

    public void addSharedSecret(String secret) {
        _secret = secret;
    }

    public void handle(String pathInContext, String pathParams, HttpRequest request, HttpResponse response)
            throws HttpException, IOException {
        if (!_hosts.contains(request.getRemoteAddr())) {
            getHttpContext().checkSecurityConstraints(pathInContext, request, response);
        } else {
            request.setAttribute(TRUSTED_SOURCE_KEY, _secret);
        }
    }
}
