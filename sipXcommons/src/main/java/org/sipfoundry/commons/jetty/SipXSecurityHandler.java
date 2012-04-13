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
package org.sipfoundry.commons.jetty;

import java.io.IOException;

import org.mortbay.http.HttpException;
import org.mortbay.http.HttpRequest;
import org.mortbay.http.HttpResponse;
import org.mortbay.http.handler.SecurityHandler;

public class SipXSecurityHandler extends SecurityHandler {
    private int m_publicHttpPort;

    public SipXSecurityHandler(int port) {
        m_publicHttpPort = port;
    }
    public void handle(String pathInContext, String pathParams, HttpRequest request, HttpResponse response)
            throws HttpException, IOException {
        if (request.getPort() == m_publicHttpPort) {
            getHttpContext().checkSecurityConstraints(pathInContext, request, response);
        }
    }
}
