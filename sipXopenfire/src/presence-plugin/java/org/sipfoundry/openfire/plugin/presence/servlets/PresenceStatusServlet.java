

package org.sipfoundry.openfire.plugin.presence.servlets;

import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.server.PropertyHandlerMapping;
import org.apache.xmlrpc.server.XmlRpcServerConfigImpl;
import org.apache.xmlrpc.webserver.XmlRpcServletServer;
import org.jivesoftware.admin.AuthCheckFilter;
import org.jivesoftware.util.Log;
import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.user.UserNotFoundException;
import org.sipfoundry.openfire.plugin.presence.SipXOpenfirePlugin;
import org.sipfoundry.openfire.plugin.presence.XmlRpcPresenceProvider;
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

public class PresenceStatusServlet extends SipXOpenfireServlet {

    public void init(ServletConfig servletConfig) throws ServletException {
        super.init(servletConfig,XmlRpcPresenceProvider.SERVER,XmlRpcPresenceProvider.SERVICE_NAME,
                XmlRpcPresenceProvider.class);
    }

  
    public void doPost(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {
        super.doPost(request,response);
    }

    
   

}
