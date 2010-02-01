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
import org.sipfoundry.openfire.plugin.presence.XmlRpcPresenceProvider;
import org.xmpp.component.ComponentManager;
import org.xmpp.component.ComponentManagerFactory;

public class SipXOpenfireServlet extends HttpServlet {
    private XmlRpcServletServer server;
    
    static SipXOpenfirePlugin plugin;
    
    org.xmpp.component.Log log ;

    private String path;
  

    public void init(ServletConfig servletConfig, String serverName, String serviceName, Class provider) throws ServletException {
        plugin = (SipXOpenfirePlugin) XMPPServer.getInstance().getPluginManager().getPlugin("sipx-openfire-presence");
        
        super.init(servletConfig);
        // Register new component
        ComponentManager componentManager = ComponentManagerFactory.getComponentManager();
     
        log = componentManager.getLog();
        log.info("initializing Servlet");
        
      
        // Exclude this servlet from requering the user to login
        this.path = "sipx-openfire-presence/" + serviceName;
        AuthCheckFilter.addExclude(path);
        
        PropertyHandlerMapping handlerMapping = new PropertyHandlerMapping();
        
        try {
            log.info("Plugin = " + plugin);
         
            handlerMapping.addHandler(serverName, provider);
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
        AuthCheckFilter.removeExclude(this.path);
    }

}
