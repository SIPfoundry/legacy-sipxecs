/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */


package org.sipfoundry.openfire.plugin.presence.servlets;

import javax.servlet.ServletConfig;
import javax.servlet.ServletException;

import org.jivesoftware.openfire.XMPPServer;
import org.sipfoundry.openfire.plugin.presence.XmlRpcPresenceProvider;

public class PresenceStatusServlet extends SipXOpenfireServlet {

    /**
     *
     */
    private static final long serialVersionUID = 1L;

    @Override
    public void init(ServletConfig servletConfig) throws ServletException {
        XMPPServer.getInstance().getPluginManager().getPlugin("sipx-openfire-presence");
        super.init(servletConfig,XmlRpcPresenceProvider.SERVER,XmlRpcPresenceProvider.SERVICE_NAME,
                XmlRpcPresenceProvider.class);
    }
}
