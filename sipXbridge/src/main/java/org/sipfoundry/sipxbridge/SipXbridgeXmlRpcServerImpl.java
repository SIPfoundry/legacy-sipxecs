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

				logger.debug("Starting xml rpc server on inetAddr:port "
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
					logger.debug("keystore = " + keystore);
					sslListener.setKeystore(keystore);
					String algorithm = System.getProperties().getProperty(
							"jetty.x509.algorithm");
					logger.debug("algorithm = " + algorithm);
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

					for (String suite : cypherSuites) {
						logger.info("Cypher Suites enabled : " + suite);
					}

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
						logger.debug("Listener = " + listener);

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

				logger.debug("Web server started.");

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

	private HashMap<String, Object> createSuccessMap() {
		HashMap<String, Object> retval = new HashMap<String, Object>();
		retval.put(STATUS_CODE, OK);
		return retval;
	}

	public Map<String, Object> getRegistrationStatus() {
		HashMap<String, Object> retval = createSuccessMap();
		try {
			HashSet<Map<String, String>> registrationRecords = new HashSet<Map<String, String>>();
			for (ItspAccountInfo itspAccount : Gateway.getAccountManager()
					.getItspAccounts()) {
				if (itspAccount.isRegisterOnInitialization()) {
					registrationRecords.add(itspAccount.getRegistrationRecord()
							.getMap());
				}
			}
			retval.put(STATUS_CODE, OK);
			retval.put(REGISTRATION_RECORDS, registrationRecords.toArray());

		} catch (Throwable ex) {
			retval.put(STATUS_CODE, ERROR);
			retval.put(ERROR_INFO, formatStackTrace(ex));
		}

		return retval;
	}

	public Map<String, Object> getCallCount() {
		HashMap<String, Object> retval = createSuccessMap();
		try {

			retval.put(STATUS_CODE, OK);
			retval.put(CALL_COUNT, new Integer(Gateway.getCallCount()))
					.toString();

		} catch (Exception ex) {
			retval.put(STATUS_CODE, ERROR);
			retval.put(ERROR_INFO, formatStackTrace(ex));
		}

		return retval;
	}

	public Map<String, Object> start() {
		HashMap<String, Object> retval = createSuccessMap();

		logger.debug("Gateway.start()");
		try {
			Gateway.start();
		} catch (Exception ex) {
			retval.put(STATUS_CODE, ERROR);
			retval.put(ERROR_INFO, formatStackTrace(ex));
		}

		return retval;
	}

	public Map<String, Object> stop() {

		logger.debug("Gateway.stop()");
		HashMap<String, Object> retval = createSuccessMap();

		try {
			Gateway.stop();
		} catch (Exception ex) {
			retval.put(STATUS_CODE, ERROR);
			retval.put(ERROR_INFO, formatStackTrace(ex));
		}
		return retval;
	}

	public Map<String, Object> exit() {
		logger.debug("Gateway.exit()");
		HashMap<String, Object> retval = createSuccessMap();

		try {

			if (Gateway.getState() == GatewayState.INITIALIZED) {
				Gateway.stop();
			}
			/*
			 * Need a fresh timer here because the gateway timer is canceled.
			 */

			new Timer().schedule(new TimerTask() {
				public void run() {
					logger.debug("Exiting bridge!");
					System.exit(0);
				}
			}, 1000);

		} catch (Exception ex) {
			retval.put(STATUS_CODE, ERROR);
			retval.put(ERROR_INFO, formatStackTrace(ex));
		}
		return retval;
	}

}
