/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxbridge;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.net.InetAddress;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;

import javax.net.ssl.SSLServerSocket;
import javax.servlet.ServletException;

import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.mortbay.http.HttpContext;
import org.mortbay.http.HttpListener;
import org.mortbay.http.HttpServer;
import org.mortbay.http.SocketListener;
import org.mortbay.http.SslListener;
import org.mortbay.jetty.servlet.ServletHandler;
import org.mortbay.util.InetAddrPort;
import org.mortbay.util.ThreadedServer;
import org.sipfoundry.sipxbridge.xmlrpc.SipXbridgeXmlRpcServer;

public class SipXbridgeXmlRpcServerImpl implements SipXbridgeXmlRpcServer {

	private static Logger logger = Logger
			.getLogger(SipXbridgeXmlRpcServerImpl.class);
	/*
	 * THe Webserver for the xml rpc interface.
	 */
	private static HttpServer webServer;

	private static boolean isWebServerRunning;

	public static void startXmlRpcServer() throws SipXbridgeException {
		try {
			if (!isWebServerRunning) {
				Logger.getLogger("org.mortbay").setLevel(Level.OFF);
				Logger.getLogger("org.apache.xmlrpc").setLevel(Level.OFF);
				isWebServerRunning = true;
				webServer = new HttpServer();

				if ( logger.isDebugEnabled() ) logger.debug("Starting xml rpc server on inetAddr:port "
						+ Gateway.getBridgeConfiguration().getLocalAddress()
						+ ":"
						+ Gateway.getBridgeConfiguration().getXmlRpcPort());
				InetAddrPort inetAddrPort = new InetAddrPort(Gateway
						.getLocalAddress(), Gateway.getBridgeConfiguration()
						.getXmlRpcPort());
				inetAddrPort.setInetAddress(InetAddress.getByName(Gateway
						.getLocalAddress()));
				if (Gateway.getBridgeConfiguration().isSecure()) {
					SslListener sslListener = new SslListener(inetAddrPort);
					inetAddrPort.setInetAddress(InetAddress.getByName(Gateway
							.getLocalAddress()));

					String keystore = System.getProperties().getProperty(
							"javax.net.ssl.keyStore");
					if ( logger.isDebugEnabled() ) logger.debug("keystore = " + keystore);
					sslListener.setKeystore(keystore);
					String algorithm = System.getProperties().getProperty(
							"jetty.x509.algorithm");
					if ( logger.isDebugEnabled() ) logger.debug("algorithm = " + algorithm);
					sslListener.setAlgorithm(algorithm);
					String password = System.getProperties().getProperty(
							"jetty.ssl.password");
					sslListener.setPassword(password);

					String keypassword = System.getProperties().getProperty(
							"jetty.ssl.keypassword");

					sslListener.setKeyPassword(keypassword);
					sslListener.setMaxThreads(32);
					sslListener.setMinThreads(4);
					sslListener.setLingerTimeSecs(30000);

					((ThreadedServer) sslListener).open();

					String[] cypherSuites = ((SSLServerSocket) sslListener
							.getServerSocket()).getSupportedCipherSuites();

					//for (String suite : cypherSuites) {
					//	logger.info("Cypher Suites enabled : " + suite);
					//}

					((SSLServerSocket) sslListener.getServerSocket())
							.setEnabledCipherSuites(cypherSuites);

					String[] protocols = ((SSLServerSocket) sslListener
							.getServerSocket()).getSupportedProtocols();

					for (String protocol : protocols) {
						logger.info("Supported protocol = " + protocol);
					}

					((SSLServerSocket) sslListener.getServerSocket())
							.setEnabledProtocols(protocols);

					webServer.setListeners(new HttpListener[] { sslListener });

					for (HttpListener listener : webServer.getListeners()) {
						if ( logger.isDebugEnabled() ) logger.debug("Listener = " + listener);

						listener.start();
					}

				} else {
					SocketListener socketListener = new SocketListener(
							inetAddrPort);
					socketListener.setMaxThreads(32);
					socketListener.setMinThreads(4);
					socketListener.setLingerTimeSecs(30000);
					webServer.addListener(socketListener);
				}

				HttpContext httpContext = new HttpContext();

				httpContext.setContextPath("/");
				ServletHandler servletHandler = new ServletHandler();
				servletHandler.addServlet("sipxbridge", "/*",
						SipxbridgeServlet.class.getName());
				httpContext.addHandler(servletHandler);

				webServer.addContext(httpContext);

				webServer.start();

				if ( logger.isDebugEnabled() ) logger.debug("Web server started.");

			}
		} catch (Exception ex) {
			throw new SipXbridgeException("Exception starting web server", ex);
		}

	}

	static void stopXmlRpcServer() {
		try {
			if (webServer != null) {
				webServer.stop();
			}
			isWebServerRunning = false;
		} catch (Exception ex) {
			logger.error("Error stopping xml rpc server.", ex);
		}
	}

	private String formatStackTrace(Throwable ex) {
		StringWriter sw = new StringWriter();
		PrintWriter pw = new PrintWriter(sw);
		ex.printStackTrace(pw);
		return sw.getBuffer().toString();
	}

	public Map<String, String> getRegistrationStatus() throws ServletException {
	    HashMap<String, String> retval = new HashMap<String, String>();
		try {
			int counter = 1;
			for (ItspAccountInfo itspAccount : Gateway.getAccountManager()
					.getItspAccounts()) {
				if (itspAccount.isRegisterOnInitialization()) {
				    retval.put(itspAccount.getRegistrationRecord().getRegisteredAddress() + " [" + itspAccount.getUserName()  + "]" ,
				            itspAccount.getRegistrationRecord().getRegistrationStatus());
				}
			}
			
		} catch (Throwable ex) {
		    throw new ServletException(formatStackTrace(ex), ex);
		}

		if ( logger.isDebugEnabled() ) logger.debug("getRegistrationStatus: " + retval);
		return retval;
	}

	public Integer getCallCount() throws ServletException {
	    int retval = 0;
		try {
			retval = Gateway.getBackToBackUserAgentFactory().getBackToBackUserAgentCount();

		} catch (Exception ex) {
		    throw new ServletException(formatStackTrace(ex), ex);
		}
		return new Integer(retval);
	}

	public Boolean start() throws ServletException {

		if ( logger.isDebugEnabled() ) logger.debug("Gateway.start()");
		try {
			Gateway.start();
		} catch (Exception ex) {
		    throw new ServletException(formatStackTrace(ex), ex);
		}

		return true;
	}

	public Boolean stop() throws ServletException {

		if ( logger.isDebugEnabled() ) logger.debug("Gateway.stop()");

		try {
			Gateway.stop();
		} catch (Exception ex) {
		    throw new ServletException(formatStackTrace(ex), ex);
		}
		return true;
	}

	public Boolean exit() throws ServletException {
		if ( logger.isDebugEnabled() ) logger.debug("Gateway.exit()");

		try {

			if (Gateway.getState() == GatewayState.INITIALIZED) {
				Gateway.stop();
			}
			/*
			 * Need a fresh timer here because the gateway timer is canceled.
			 */

			new Timer().schedule(new TimerTask() {
				public void run() {
					if ( logger.isDebugEnabled() ) logger.debug("Exiting bridge!");
					System.exit(0);
				}
			}, 1000);

		} catch (Exception ex) {
		    throw new ServletException(formatStackTrace(ex), ex);
		}
		return true;
	}

}
