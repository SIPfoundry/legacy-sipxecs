package org.sipfoundry.openfire.plugin.presence.servlets;

import java.io.IOException;

import javax.servlet.ServletConfig;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.server.PropertyHandlerMapping;
import org.apache.xmlrpc.server.XmlRpcServerConfigImpl;
import org.apache.xmlrpc.webserver.XmlRpcServletServer;
import org.jivesoftware.admin.AuthCheckFilter;
import org.jivesoftware.openfire.XMPPServer;
import org.sipfoundry.openfire.plugin.presence.SipXOpenfirePlugin;
import org.sipfoundry.openfire.plugin.presence.XmlRpcUserAccountProvider;
import org.xmpp.component.ComponentManager;
import org.xmpp.component.ComponentManagerFactory;

public class UserAccountServlet extends SipXOpenfireServlet {
    private XmlRpcServletServer server;

    static SipXOpenfirePlugin plugin;

    org.xmpp.component.Log log;

    public void init(ServletConfig servletConfig) throws ServletException {
        super.init(servletConfig,XmlRpcUserAccountProvider.SERVER,XmlRpcUserAccountProvider.SERVICE,XmlRpcUserAccountProvider.class);
       
    }

   
}
