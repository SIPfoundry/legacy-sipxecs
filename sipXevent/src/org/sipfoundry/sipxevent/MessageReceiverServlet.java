/**
 *
 *
 * Copyright (c) 2010 / 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxevent;

import java.io.IOException;
import java.io.InputStream;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.io.IOUtils;
import org.apache.log4j.Logger;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.web.HttpRequestHandler;

public class MessageReceiverServlet implements HttpRequestHandler {
    private RegisteredClients m_registeredClients;
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxevent");

    @Override
    public void handleRequest(HttpServletRequest request, HttpServletResponse response) throws ServletException,
            IOException {

        InputStream in = request.getInputStream();
        byte[] message = IOUtils.toByteArray(in);
        String messageString = new String(message);
        IOUtils.closeQuietly(in);
        String userName = request.getHeader("user_id");
        SipXWebSocket webSocket = m_registeredClients.getWebSocket(userName);
        if (webSocket != null) {
            //webSocket.getConnection().sendMessage(messageString);
            LOG.debug("Message sent: "+messageString + " to user: "+userName);
        }
    }

    @Required
    public void setRegisteredClients(RegisteredClients registeredClients) {
        m_registeredClients = registeredClients;
    }
}
