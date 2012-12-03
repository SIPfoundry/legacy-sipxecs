/**
 *
 *
 * Copyright (c) 2010 / 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxevent;

import java.util.Properties;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;
import org.eclipse.jetty.server.Connector;
import org.eclipse.jetty.server.Server;
import org.eclipse.jetty.server.ssl.SslSelectChannelConnector;
import org.eclipse.jetty.util.ssl.SslContextFactory;
import org.eclipse.jetty.webapp.WebAppContext;
import org.sipfoundry.commons.log4j.SipFoundryLayout;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.support.ClassPathXmlApplicationContext;

public class WebSocketServer {
    private String m_keystore;
    private String m_password;
    private String m_logFile;
    private String m_resourceBase;
    private String m_logLevel;
    private String m_confDir;
    private int m_port;

    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxevent");

    public void init() {
        // Configure log4j
        Properties props = new Properties();
        props.setProperty("log4j.rootLogger", "warn, file");
        props.setProperty("log4j.logger.org.sipfoundry.sipxwebsocket",
                SipFoundryLayout.mapSipFoundry2log4j(m_logLevel).toString());
        props.setProperty("log4j.appender.file", "org.sipfoundry.commons.log4j.SipFoundryAppender");
        props.setProperty("log4j.appender.file.File", m_logFile);
        props.setProperty("log4j.appender.file.layout", "org.sipfoundry.commons.log4j.SipFoundryLayout");
        props.setProperty("log4j.appender.file.layout.facility", "sipXwebsocket");
        PropertyConfigurator.configure(props);
    }

	public void start() {
		Server server = new Server();
		try {
		    SslSelectChannelConnector sslConnector =  new SslSelectChannelConnector();
		    sslConnector.setPort(m_port);

		    SslContextFactory cf = sslConnector.getSslContextFactory();
		    configureContextFactory(cf);

	        server.setConnectors(new Connector[]{sslConnector});

	        WebAppContext webapp = new WebAppContext();
	        webapp.setDescriptor(m_resourceBase + "/WEB-INF/web.xml");
	        webapp.setResourceBase(m_resourceBase);
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
        cf.setKeyStore(m_keystore);
        cf.setKeyStorePassword(m_password);
    }

	public static void main(String [] args) {
	    ClassPathXmlApplicationContext ctx = new ClassPathXmlApplicationContext("server.beans.xml");
        WebSocketServer server = (WebSocketServer) ctx.getBean("webSocketServer");

        server.start();
	}

    @Required
    public void setKeystore(String keystore) {
        m_keystore = keystore;
    }

    @Required
    public void setPassword(String password) {
        m_password = password;
    }

    @Required
    public void setPort(int port) {
        m_port = port;
    }

    @Required
    public void setLogFile(String logFile) {
        m_logFile = logFile;
    }

    @Required
    public void setLogLevel(String logLevel) {
        m_logLevel = logLevel;
    }

    @Required
    public void setResourceBase(String resourceBase) {
        m_resourceBase = resourceBase;
    }

    @Required
    public void setConfDir(String confDir) {
        m_confDir = confDir;
    }

    public String getConfDir() {
        return m_confDir;
    }
}
