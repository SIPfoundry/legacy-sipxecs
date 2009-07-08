package org.sipfoundry.openfire.plugin.presence;

import javax.servlet.ServletConfig;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;

import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.server.PropertyHandlerMapping;
import org.apache.xmlrpc.server.XmlRpcServerConfigImpl;
import org.apache.xmlrpc.webserver.XmlRpcServletServer;
import org.jivesoftware.admin.AuthCheckFilter;
import org.jivesoftware.openfire.XMPPServer;
import org.xmpp.component.ComponentManager;
import org.xmpp.component.ComponentManagerFactory;

public class OpenfireServerControlServlet extends HttpServlet {
    private XmlRpcServletServer server;
    
    static SipXOpenfirePlugin plugin;
    
    org.xmpp.component.Log log ;
  
    public void init(ServletConfig servletConfig) throws ServletException {
        super.init(servletConfig);
        // Register new component
        ComponentManager componentManager = ComponentManagerFactory.getComponentManager();

        log = componentManager.getLog();
        log.info("initializing UserAccountServlet");

        plugin = (SipXOpenfirePlugin) XMPPServer.getInstance().getPluginManager().getPlugin(
                "sipx-openfire");

        // Exclude this servlet from requering the user to login
        AuthCheckFilter.addExclude("sipx-openfire/serverctl");

        PropertyHandlerMapping handlerMapping = new PropertyHandlerMapping();

        try {
            log.info("Plugin = " + plugin);
            XmlRpcUserAccountProvider.plugin = plugin;
            handlerMapping
                    .addHandler(XmlRpcServerControlProvider.SERVER, XmlRpcServerControlProvider.class);
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


}
