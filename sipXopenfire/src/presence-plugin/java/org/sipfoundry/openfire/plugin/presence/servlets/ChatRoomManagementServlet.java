/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.plugin.presence.servlets;

import java.io.IOException;

import javax.servlet.ServletConfig;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.sipfoundry.openfire.plugin.presence.XmlRpcChatRoomManagementProvider;

public class ChatRoomManagementServlet extends SipXOpenfireServlet{
    public void init(ServletConfig servletConfig) throws ServletException {
        super.init(servletConfig, 
                XmlRpcChatRoomManagementProvider.SERVER, 
                XmlRpcChatRoomManagementProvider.SERVICE_NAME,
                XmlRpcChatRoomManagementProvider.class);
    }
    
    public void doPost(HttpServletRequest request, HttpServletResponse response)
    throws ServletException, IOException {
        super.doPost(request,response);
    }


}
