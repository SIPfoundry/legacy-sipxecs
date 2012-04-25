/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */


package org.sipfoundry.openfire.plugin.presence.servlets;

import org.jivesoftware.openfire.XMPPServer;
import org.sipfoundry.openfire.plugin.presence.XmlRpcPresenceProvider;

import javax.servlet.ServletConfig;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;

public class PresenceStatusServlet extends SipXOpenfireServlet {

    public void init(ServletConfig servletConfig) throws ServletException {
        XMPPServer.getInstance().getPluginManager().getPlugin("sipx-openfire-presence");
        super.init(servletConfig,XmlRpcPresenceProvider.SERVER,XmlRpcPresenceProvider.SERVICE_NAME,
                XmlRpcPresenceProvider.class);
    }

  
    public void doPost(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {
        super.doPost(request,response);
    }

    
   

}
