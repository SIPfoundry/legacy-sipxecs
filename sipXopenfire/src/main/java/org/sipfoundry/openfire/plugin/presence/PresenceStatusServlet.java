/**
 * $RCSfile$
 * $Revision: 1710 $
 * $Date: 2005-07-26 15:56:14 -0300 (Tue, 26 Jul 2005) $
 *
 * Copyright (C) 2004-2008 Jive Software. All rights reserved.
 *
 * This software is published under the terms of the GNU Public License (GPL),
 * a copy of which is included in this distribution, or a commercial license
 * agreement with Jive.
 */

package org.sipfoundry.openfire.plugin.presence;

import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.server.PropertyHandlerMapping;
import org.apache.xmlrpc.server.XmlRpcServerConfigImpl;
import org.apache.xmlrpc.webserver.XmlRpcServletServer;
import org.jivesoftware.admin.AuthCheckFilter;
import org.jivesoftware.util.Log;
import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.user.UserNotFoundException;
import org.xmpp.component.ComponentManager;
import org.xmpp.component.ComponentManagerFactory;
import org.xmpp.packet.Presence;

import javax.servlet.ServletConfig;
import javax.servlet.ServletContext;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;

import org.jivesoftware.openfire.container.Plugin;

public class PresenceStatusServlet extends HttpServlet {
    
    private XmlRpcServletServer server;
 
    static SipXOpenfirePlugin plugin;
    
    org.xmpp.component.Log log ;
  

    public void init(ServletConfig servletConfig) throws ServletException {
        super.init(servletConfig);
        // Register new component
        ComponentManager componentManager = ComponentManagerFactory.getComponentManager();
     
        log = componentManager.getLog();
        log.info("initializing PresenceStatusServlet");
        
        plugin =
                (SipXOpenfirePlugin) XMPPServer.getInstance().getPluginManager().getPlugin("sipx-openfire");
      
        // Exclude this servlet from requering the user to login
        AuthCheckFilter.addExclude("sipx-openfire/status");
        
        PropertyHandlerMapping handlerMapping = new PropertyHandlerMapping();
        
        try {
            log.info("Plugin = " + plugin);
            XmlRpcPresenceProvider.plugin = plugin;
            handlerMapping.addHandler(XmlRpcPresenceProvider.SERVER, XmlRpcPresenceProvider.class);
        } catch (XmlRpcException e) {
           throw new ServletException("XmlRpcInitialization failed");
        }
        
        server = new XmlRpcServletServer();
        

        XmlRpcServerConfigImpl serverConfig = new XmlRpcServerConfigImpl();
        serverConfig.setKeepAliveEnabled(true);
        serverConfig.setEnabledForExceptions(true);
        serverConfig.setEnabledForExtensions(true);
        server.setMaxThreads(4);

        server.setConfig(serverConfig);
        server.setHandlerMapping(handlerMapping);
    }

  
    protected void doPost(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {
        server.execute(request,response);
    }

    public void destroy() {
        super.destroy();
        
        // Release the excluded URL
        AuthCheckFilter.removeExclude("sipx-openfire/status");
    }

   

}
