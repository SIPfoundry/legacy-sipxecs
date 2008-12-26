/*
 * 
 * Copyright (C) 2008 Nortel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.sipxbridge;

import java.io.IOException;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.xmlrpc.server.PropertyHandlerMapping;
import org.apache.xmlrpc.server.XmlRpcServerConfigImpl;
import org.apache.xmlrpc.webserver.XmlRpcServletServer;
import org.sipfoundry.sipxbridge.xmlrpc.SipXbridgeXmlRpcServer;

/**
 * The servlet class that routes XML RPC requests for sipxbridge to 
 * the appropriate handler.
 */
@SuppressWarnings("serial")
public class SipxbridgeServlet extends HttpServlet {
	 private XmlRpcServletServer server;


	    /*
	     * (non-Javadoc)
	     * 
	     * @see javax.servlet.GenericServlet#init()
	     */
	    public void init() throws ServletException {
	    	 try {
	            
	             PropertyHandlerMapping handlerMapping = new PropertyHandlerMapping();

	             handlerMapping.addHandler(SipXbridgeXmlRpcServer.SERVER, SipXbridgeXmlRpcServerImpl.class);

	             server = new XmlRpcServletServer();
	            

	             XmlRpcServerConfigImpl serverConfig = new XmlRpcServerConfigImpl();
	             serverConfig.setKeepAliveEnabled(true);
	             serverConfig.setEnabledForExceptions(true);
	             serverConfig.setEnabledForExtensions(true);
	             server.setMaxThreads(4);

	             server.setConfig(serverConfig);
	             server.setHandlerMapping(handlerMapping);
	         } catch (Exception ex) {
	             throw new ServletException("Initialization failure", ex);
	         }
	    }

	    /*
	     * (non-Javadoc)
	     * 
	     * @see javax.servlet.http.HttpServlet#doPost(javax.servlet.http.HttpServletRequest,
	     *      javax.servlet.http.HttpServletResponse)
	     */
	    public void doPost(HttpServletRequest request, HttpServletResponse response)
	            throws ServletException, IOException {
	        this.server.execute(request, response);

	    }

}
