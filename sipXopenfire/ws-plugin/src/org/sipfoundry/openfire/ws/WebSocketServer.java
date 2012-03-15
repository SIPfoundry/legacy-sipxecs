/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.openfire.ws;

import java.io.File;

import org.eclipse.jetty.server.Connector;
import org.eclipse.jetty.server.Server;
import org.eclipse.jetty.server.ssl.SslSelectChannelConnector;
import org.eclipse.jetty.util.ssl.SslContextFactory;
import org.eclipse.jetty.webapp.WebAppContext;

public class WebSocketServer {
	private String m_rootPath;

	public WebSocketServer(String rootPath) {
		m_rootPath = rootPath;
	}
	public void start() {
		Server server = new Server();
		try {
		    SslSelectChannelConnector sslConnector =  new SslSelectChannelConnector();
		    sslConnector.setPort(8082);

		    SslContextFactory cf = sslConnector.getSslContextFactory();
		    configureContextFactory(cf);

	        server.setConnectors(new Connector[]{sslConnector});

	        WebAppContext webapp = new WebAppContext();
	        webapp.setDescriptor(m_rootPath + File.separator + "web/WEB-INF/web.xml");
	        webapp.setResourceBase(m_rootPath);
	        webapp.setContextPath("/");
	        server.setHandler(webapp);

			server.start();
			server.join();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

    private void configureContextFactory(SslContextFactory cf) throws Exception {
        cf.setProtocol("SSLv3");
        String keystore = System.getProperties().getProperty("javax.net.ssl.keyStore");
        System.out.println("keystore = " + keystore);
        cf.setKeyStore("/usr/local/sipx/etc/sipxpbx/ssl/ssl-web.keystore");

        //String password = System.getProperties().getProperty("jetty.ssl.keypassword");
        //System.out.println("password = " + password);
        cf.setKeyStorePassword("changeit");
    }


	public static void main(String [] args) {
		(new WebSocketServer(".")).start();
	}
}
