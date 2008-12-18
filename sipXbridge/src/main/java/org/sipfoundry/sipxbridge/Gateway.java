/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.SipStackExt;
import gov.nist.javax.sip.clientauthutils.AuthenticationHelper;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Properties;
import java.util.Timer;
import java.util.TimerTask;
import java.util.logging.Handler;
import java.util.logging.Level;

import javax.sip.Dialog;
import javax.sip.ListeningPoint;
import javax.sip.SipProvider;
import javax.sip.address.Address;
import javax.sip.address.SipURI;
import javax.sip.message.Request;

import net.java.stun4j.StunAddress;
import net.java.stun4j.client.NetworkConfigurationDiscoveryProcess;
import net.java.stun4j.client.StunDiscoveryReport;

import org.apache.log4j.Logger;
import org.mortbay.http.HttpContext;
import org.mortbay.http.HttpListener;
import org.mortbay.http.HttpServer;
import org.mortbay.http.SocketListener;
import org.mortbay.http.SslListener;
import org.mortbay.jetty.servlet.ServletHandler;
import org.mortbay.util.InetAddrPort;
import org.mortbay.util.ThreadedServer;
import org.sipfoundry.commons.log4j.SipFoundryAppender;
import org.sipfoundry.commons.log4j.SipFoundryLayout;
import org.sipfoundry.sipxbridge.symmitron.SymmitronClient;
import org.sipfoundry.sipxbridge.symmitron.SymmitronConfig;
import org.sipfoundry.sipxbridge.symmitron.SymmitronConfigParser;
import org.sipfoundry.sipxbridge.symmitron.SymmitronException;
import org.xbill.DNS.Lookup;
import org.xbill.DNS.Record;
import org.xbill.DNS.SRVRecord;
import org.xbill.DNS.Type;

/**
 * The main class
 * 
 * @author M. Ranganathan
 * 
 */
public class Gateway {

	private static Logger logger = Logger.getLogger(Gateway.class);

	private static String configurationFile = "file:///etc/sipxpbx/sipxbridge.xml";

	static final String SIPXBRIDGE_USER = "sipxbridge";
	/*
	 * The account manager -- tracks user accounts. This is populated by reading
	 * the sipxbridge.xml configuration file.
	 */
	private static AccountManagerImpl accountManager;

	/*
	 * The security manager - handles wrapping credentials etc.
	 */
	private static AuthenticationHelper authenticationHelper;

	/*
	 * The registration manager.
	 */
	private static RegistrationManager registrationManager;

	/*
	 * Internal SIp Provider
	 */
	private static SipProvider internalProvider;

	/*
	 * External provider.
	 */
	private static SipProvider externalProvider;

	/*
	 * External
	 */
	private static SipProvider externalTlsProvider;

	/*
	 * CallControl router.
	 */
	private static CallControlManager callControlManager;

	/*
	 * The allocator for back to back user agents.
	 */

	private static BackToBackUserAgentFactory backToBackUserAgentFactory;

	/*
	 * This is a placeholder - to be replaced by STUN
	 */
	private static String globalAddress;

	/*
	 * The stun port
	 */
	private static final int STUN_PORT = 3478;

	/*
	 * A table of timers for re-registration
	 */
	private static Timer timer = new Timer();

	/*
	 * The Music on hold URL
	 */
	private static Address musicOnHoldAddress;

	/*
	 * The From address of the gateway.
	 */
	private static Address gatewayFromAddress;

	/*
	 * THe Webserver for the xml rpc interface.
	 */
	private static HttpServer webServer;

	/*
	 * The Gateway state.
	 */
	static GatewayState state = GatewayState.STOPPED;

	/*
	 * The time for REGISTER requests.
	 */
	private static final int MAX_REGISTRATION_TIMER = 10000;

	/*
	 * Default transport to talk to ITSP
	 */
	protected static final String DEFAULT_ITSP_TRANSPORT = "udp";

	/*
	 * Min value for session timer ( seconds ).
	 */
	protected static final int MIN_EXPIRES = 60;

	/*
	 * Session expires interval (initial value)
	 */
	protected static final int SESSION_EXPIRES = 30 * 60;

	/*
	 * Advance timer by 10 seconds for session timer.
	 */
	protected static final int TIMER_ADVANCE = 10;

	/*
	 * set to true to enable tls for sip signaling (untested).
	 */

	private static boolean isTlsSupportEnabled = false;

	private static int callCount;

	private static boolean isWebServerRunning;

	private static Hashtable<String, SymmitronClient> symmitronClients = new Hashtable<String, SymmitronClient>();

	private static String configurationPath;

	private static HashSet<String> proxyAddressTable = new HashSet<String>();

	private static HashSet<Integer> parkServerCodecs = new HashSet<Integer>();

	static SipFoundryAppender logAppender;



	private Gateway() {

	}

	static void setConfigurationFileName(String configFileName) {
		Gateway.configurationFile = configFileName;
	}

	static void parseConfigurationFile() {
		ConfigurationParser parser = new ConfigurationParser();
		accountManager = parser.createAccountManager(configurationFile);

	}

	static SymmitronClient initializeSymmitron(String address) {
		SymmitronClient symmitronClient = symmitronClients.get(address);
		if (symmitronClients != null) {
			int symmitronPort;
			boolean isSecure = false;
			if (Gateway.getBridgeConfiguration().getSymmitronXmlRpcPort() != 0) {
				symmitronPort = Gateway.getBridgeConfiguration()
						.getSymmitronXmlRpcPort();
			} else {
				SymmitronConfig symconfig = new SymmitronConfigParser()
						.parse(Gateway.configurationPath
								+ "/nattraversalrules.xml");
				symmitronPort = symconfig.getXmlRpcPort();
				isSecure = symconfig.getUseHttps();
			}
		
			symmitronClient = new SymmitronClient(address, symmitronPort, isSecure,
					callControlManager);
		}
		symmitronClients.put(address, symmitronClient);

		return symmitronClient;
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

	public static void startXmlRpcServer() throws GatewayConfigurationException {
		try {
			if (!isWebServerRunning) {

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
			throw new GatewayConfigurationException(
					"Exception starting web server", ex);
		}

	}

	/**
	 * Initialize the loggers for the libraries used.
	 * 
	 * @throws IOException
	 */
	static void initializeLogging() throws IOException {

		BridgeConfiguration bridgeConfiguration = Gateway
				.getBridgeConfiguration();
		Level level = Level.OFF;
		String logLevel = bridgeConfiguration.getLogLevel();

		if (logLevel.equals("INFO"))
			level = Level.INFO;
		else if (logLevel.equals("DEBUG"))
			level = Level.FINE;
		else if (logLevel.equals("TRACE"))
			level = Level.FINER;
		else if (logLevel.equals("WARN"))
			level = Level.WARNING;

		/*
		 * BUGBUG For now turn off Logging on STUN4j. It writes to stdout.
		 */
		level = Level.OFF;

		java.util.logging.Logger log = java.util.logging.Logger
				.getLogger("net.java.stun4j");
		log.setLevel(level);
		java.util.logging.FileHandler fileHandler = new java.util.logging.FileHandler(
				Gateway.getLogFile());

		/*
		 * Remove all existing handlers.
		 */
		for (Handler handler : log.getHandlers()) {
			log.removeHandler(handler);
		}

		/*
		 * Add the file handler.
		 */
		log.addHandler(fileHandler);

		Gateway.logAppender = new SipFoundryAppender(new SipFoundryLayout(),
				Gateway.getLogFile());
		Logger applicationLogger = Logger.getLogger(Gateway.class.getPackage()
				.getName());

		/*
		 * Set the log level.
		 */
		if (Gateway.getLogLevel().equals("TRACE")) {
			applicationLogger.setLevel(org.apache.log4j.Level.DEBUG);
		} else {
			applicationLogger.setLevel(org.apache.log4j.Level.toLevel(Gateway
					.getLogLevel()));
		}

		applicationLogger.addAppender(logAppender);

	}

	/**
	 * Discover our address using stun.
	 * 
	 * @throws GatewayConfigurationException
	 */
	static void discoverAddress() throws GatewayConfigurationException {
		try {

			BridgeConfiguration bridgeConfiguration = accountManager
					.getBridgeConfiguration();
			String stunServerAddress = bridgeConfiguration
					.getStunServerAddress();

			if (stunServerAddress != null) {

				java.util.logging.Logger log = java.util.logging.Logger
						.getLogger("net.java.stun4j");

				// Todo -- deal with the situation when this port may be taken.
				StunAddress localStunAddress = new StunAddress(Gateway
						.getLocalAddress(), STUN_PORT);

				StunAddress serverStunAddress = new StunAddress(
						stunServerAddress, STUN_PORT);

				NetworkConfigurationDiscoveryProcess addressDiscovery = new NetworkConfigurationDiscoveryProcess(
						localStunAddress, serverStunAddress);

				addressDiscovery.start();
				StunDiscoveryReport report = addressDiscovery
						.determineAddress();
				globalAddress = report.getPublicAddress().getSocketAddress()
						.getAddress().getHostAddress();
				logger.debug("Stun report = " + report);
				if (report.getPublicAddress().getPort() != STUN_PORT) {
					logger.debug("WARNING External port != internal port ");
				}

			}
		} catch (Exception ex) {
			throw new GatewayConfigurationException(
					"Error discovering  address", ex);
		}
	}

	/**
	 * Start timer to rediscover our address.
	 * 
	 */
	static void startRediscoveryTimer() {
		int rediscoveryTime = Gateway.accountManager.getBridgeConfiguration()
				.getGlobalAddressRediscoveryPeriod();
		TimerTask ttask = new TimerTask() {

			@Override
			public void run() {
				try {
					Gateway.discoverAddress();
				} catch (Exception ex) {
					logger.error("Error re-discovering  address");
				}

			}

		};
		Gateway.getTimer().schedule(ttask, rediscoveryTime * 1000,
				rediscoveryTime * 1000);
	}

	/**
	 * Get the proxy addresses. This is done once at startup. We cannot accecpt
	 * inbound INVITE from other addresses than the ones on this list.
	 */

	static void getSipxProxyAddresses() throws GatewayConfigurationException {
		try {

			Record[] records = new Lookup("_sip._" + getSipxProxyTransport()
					+ "." + getSipxProxyDomain(), Type.SRV).run();

			/* repopulate the proxy address table in case of reconfiguration */
			Gateway.proxyAddressTable.clear();
			if (records == null) {
				logger.debug("SRV record lookup returned null");
				String sipxProxyAddress = getSipxProxyDomain();
				Gateway.proxyAddressTable.add(InetAddress.getByName(
						sipxProxyAddress).getHostAddress());
			} else {
				for (Record rec : records) {
					SRVRecord record = (SRVRecord) rec;
					String resolvedName = record.getTarget().toString();
					/*
					 * Sometimes the DNS (on linux) appends a "." to the end of
					 * the record. The library ought to strip this.
					 */
					if (resolvedName.endsWith(".")) {
						resolvedName = resolvedName.substring(0, resolvedName
								.lastIndexOf('.'));
					}
					String ipAddress = InetAddress.getByName(resolvedName)
							.getHostAddress();

					Gateway.proxyAddressTable.add(ipAddress);
				}
			}

			logger.debug("proxy address table = " + proxyAddressTable);
		} catch (Exception ex) {
			logger.error("Cannot do address lookup ", ex);
			throw new GatewayConfigurationException(
					"Could not do dns lookup for " + getSipxProxyDomain(), ex);
		}
	}

	static boolean isAddressFromProxy(String address) {

		return proxyAddressTable.contains(address);

	}

	/**
	 * Initialize the bridge.
	 * 
	 */
	static void initializeSipListeningPoints() {
		try {

			BridgeConfiguration bridgeConfiguration = accountManager
					.getBridgeConfiguration();

			authenticationHelper = ((SipStackExt) ProtocolObjects.sipStack)
					.getAuthenticationHelper(accountManager,
							ProtocolObjects.headerFactory);

			int externalPort = bridgeConfiguration.getExternalPort();
			String externalAddress = bridgeConfiguration.getExternalAddress();
			logger.debug("External Address:port = " + externalAddress + ":"
					+ externalPort);
			ListeningPoint externalUdpListeningPoint = ProtocolObjects.sipStack
					.createListeningPoint(externalAddress, externalPort, "udp");
			ListeningPoint externalTcpListeningPoint = ProtocolObjects.sipStack
					.createListeningPoint(externalAddress, externalPort, "tcp");
			if (Gateway.isTlsSupportEnabled) {
				ListeningPoint externalTlsListeningPoint = ProtocolObjects.sipStack
						.createListeningPoint(externalAddress,
								externalPort + 1, "tls");
				externalTlsProvider = ProtocolObjects.sipStack
						.createSipProvider(externalTlsListeningPoint);
			}
			externalProvider = ProtocolObjects.sipStack
					.createSipProvider(externalUdpListeningPoint);
			externalProvider.addListeningPoint(externalTcpListeningPoint);

			int localPort = bridgeConfiguration.getLocalPort();
			String localIpAddress = bridgeConfiguration.getLocalAddress();

			SipURI mohUri = accountManager.getBridgeConfiguration()
					.getMusicOnHoldName() == null ? null
					: ProtocolObjects.addressFactory.createSipURI(
							accountManager.getBridgeConfiguration()
									.getMusicOnHoldName(), Gateway
									.getSipxProxyDomain());
			if (mohUri != null) {
				musicOnHoldAddress = ProtocolObjects.addressFactory
						.createAddress(mohUri);
			}

			String domain = Gateway.getSipxProxyDomain();
			gatewayFromAddress = ProtocolObjects.addressFactory
					.createAddress(ProtocolObjects.addressFactory.createSipURI(
							SIPXBRIDGE_USER, domain));
			logger.debug("Local Address:port " + localIpAddress + ":"
					+ localPort);

			ListeningPoint internalUdpListeningPoint = ProtocolObjects.sipStack
					.createListeningPoint(localIpAddress, localPort, "udp");

			ListeningPoint internalTcpListeningPoint = ProtocolObjects.sipStack
					.createListeningPoint(localIpAddress, localPort, "tcp");

			internalProvider = ProtocolObjects.sipStack
					.createSipProvider(internalUdpListeningPoint);

			internalProvider.addListeningPoint(internalTcpListeningPoint);

			registrationManager = new RegistrationManager(getWanProvider("udp"));

			callControlManager = new CallControlManager();

			backToBackUserAgentFactory = new BackToBackUserAgentFactory();

		} catch (Throwable ex) {
			ex.printStackTrace();
			logger.error("Cannot initialize gateway", ex);
			throw new GatewayConfigurationException(
					"Cannot initialize gateway", ex);
		}

	}

	static AccountManagerImpl getAccountManager() {
		return Gateway.accountManager;

	}

	static BridgeConfiguration getBridgeConfiguration() {
		return accountManager.getBridgeConfiguration();
	}

	static RegistrationManager getRegistrationManager() {
		return registrationManager;
	}

	static AuthenticationHelper getAuthenticationHelper() {
		return authenticationHelper;
	}

	static SipProvider getWanProvider(String transport) {
		if (transport.equalsIgnoreCase("tls"))
			return externalTlsProvider;
		else
			return externalProvider;
	}

	static SipProvider getLanProvider() {
		return internalProvider;
	}

	/**
	 * @return the call control router.
	 */
	static CallControlManager getCallControlManager() {
		return callControlManager;
	}

	/**
	 * Get the back to back user agent factory.
	 * 
	 * @return the back to back user agent factory.
	 */
	static BackToBackUserAgentFactory getBackToBackUserAgentFactory() {
		return backToBackUserAgentFactory;
	}

	/**
	 * The local address of the gateway.
	 * 
	 * @return
	 */

	static String getLocalAddress() {
		return accountManager.getBridgeConfiguration().getLocalAddress();
	}

	/**
	 * The transport to use to talk to sipx proxy. This is registered in the DNS
	 * srv.
	 * 
	 * @return the proxy transport
	 */
	static String getSipxProxyTransport() {
		return accountManager.getBridgeConfiguration().getSipxProxyTransport();
	}

	/**
	 * The Sipx proxy domain.
	 * 
	 * @return the sipx proxy domain name.
	 */
	static String getSipxProxyDomain() {
		return accountManager.getBridgeConfiguration().getSipxProxyDomain();
	}

	/**
	 * 
	 * @return the bridge log level.
	 */
	static String getLogLevel() {
		return accountManager.getBridgeConfiguration().getLogLevel();
	}

	/**
	 * Get the timeout for media.
	 * 
	 * @return
	 */
	static int getMediaKeepaliveMilisec() {

		return Gateway.accountManager.getBridgeConfiguration()
				.getMediaKeepalive();
	}

	/**
	 * Get the sip keepalive
	 * 
	 * @return
	 */
	static int getSipKeepaliveSeconds() {
		return Gateway.accountManager.getBridgeConfiguration()
				.getSipKeepalive();
	}

	/**
	 * Get the MOH server Request URI.
	 */
	static SipURI getMusicOnHoldUri() {
		try {
			return accountManager.getBridgeConfiguration().getMusicOnHoldName() == null ? null
					: ProtocolObjects.addressFactory.createSipURI(
							accountManager.getBridgeConfiguration()
									.getMusicOnHoldName(), Gateway
									.getSipxProxyDomain());
		} catch (Exception ex) {
			logger.error("Unexpected exception creating Music On Hold URI", ex);
			throw new RuntimeException("Unexpected exception", ex);
		}
	}

	/**
	 * @return the Music On Hold server Address.
	 */
	static Address getMusicOnHoldAddress() {
		return musicOnHoldAddress;
	}

	/**
	 * Get the Gateway Address ( used in From Header ) of requests that
	 * originate from the Gateway.
	 * 
	 * @return an address that can be used in the From Header of request that
	 *         originate from the Gateway.
	 */
	static Address getGatewayFromAddress() {
		return Gateway.gatewayFromAddress;

	}

	static void registerWithItsp() throws GatewayConfigurationException {
		logger.info("------- REGISTERING--------");
		try {
			Gateway.accountManager.lookupItspAccountAddresses();
			Gateway.accountManager.startAuthenticationFailureTimers();

			for (ItspAccountInfo itspAccount : Gateway.accountManager
					.getItspAccounts()) {

				if (itspAccount.isRegisterOnInitialization()
						&& itspAccount.getState() != AccountState.INVALID) {
					Gateway.registrationManager.sendRegistrer(itspAccount);
				}

			}

			/*
			 * Wait for successful registration.
			 */
			for (int i = 0; i < MAX_REGISTRATION_TIMER / 1000; i++) {

				try {
					Thread.sleep(1000);
				} catch (InterruptedException ex) {
					throw new RuntimeException(
							"Unexpected exception registering", ex);
				}

				/*
				 * Check all accounts - if they are Authenticated we are done.
				 */
				boolean allAuthenticated = true;
				for (ItspAccountInfo itspAccount : Gateway.accountManager
						.getItspAccounts()) {
					if (itspAccount.isRegisterOnInitialization()
							&& itspAccount.getState() == AccountState.AUTHENTICATING) {
						allAuthenticated = false;
						break;
					}
				}
				if (allAuthenticated)
					break;

			}

			/*
			 * For all those who ask for keepalive and don't need registration,
			 * kick off their timers.
			 */
			for (ItspAccountInfo itspAccount : Gateway.getAccountManager()
					.getItspAccounts()) {
				if (!itspAccount.isRegisterOnInitialization()
						&& itspAccount.getSipKeepaliveMethod().equals("CR-LF")
						&& itspAccount.getState() != AccountState.INVALID) {
					itspAccount.startCrLfTimerTask();

				}
			}

		} catch (GatewayConfigurationException ex) {
			logger.fatal(ex);
			throw ex;

		} catch (Exception ex) {
			logger.fatal(ex);
			throw new GatewayConfigurationException(ex.getMessage());
		}
	}

	/**
	 * Registers all listeners and starts everything.
	 * 
	 * @throws Exception
	 */
	static void startSipListener() throws GatewayConfigurationException {

		try {
			SipListenerImpl listener = new SipListenerImpl();
			getWanProvider("udp").addSipListener(listener);
			if (Gateway.isTlsSupportEnabled)
				getWanProvider("tls").addSipListener(listener);
			getLanProvider().addSipListener(listener);
			ProtocolObjects.start();
		} catch (Exception ex) {
			throw new GatewayConfigurationException(
					"Could not start gateway -- check configuration", ex);
		}

	}

	static void startAddressDiscovery() {

		if (Gateway.getGlobalAddress() == null
				&& Gateway.accountManager.getBridgeConfiguration()
						.getStunServerAddress() == null) {
			throw new GatewayConfigurationException(
					"Gateway address or stun server required. ");
		}

		if (Gateway.getGlobalAddress() == null) {
			discoverAddress();
			startRediscoveryTimer();
		} else {
			Gateway.accountManager.getBridgeConfiguration()
					.setStunServerAddress(null);
		}

	}

	static void start() throws GatewayConfigurationException {
		if (Gateway.getState() != GatewayState.STOPPED) {
			return;
		}
		Gateway.state = GatewayState.INITIALIZING;

		/*
		 * If specified, try to contact symmitron.
		 */
		if (Gateway.getBridgeConfiguration().getSymmitronHost() != null) {
			int i;
			for (i = 0; i < 8; i++) {
				try {
					Gateway.initializeSymmitron(Gateway
							.getBridgeConfiguration().getSymmitronHost());
				} catch (SymmitronException ex) {
					logger.error("Symmitron not started -- retrying!");
					try {
						Thread.sleep(30000);
						continue;
					} catch (InterruptedException e) {

					}

				}
				break;
			}
			if (i == 8) {
				logger
						.fatal("Coould not contact symmitron - please start symmitron on "
								+ Gateway.getBridgeConfiguration()
										.getSymmitronHost());

			}
		}

		/*
		 * Initialize the JAIN-SIP listening points.
		 */
		initializeSipListeningPoints();

		/*
		 * Lookup the proxy addresses and keep them cached. Note that this is
		 * done just once. sipxbridge will need to be restarted if the DNS
		 * records are re-configured. We relup upon sipxsupervisor restarting
		 * sipxbridge when dns addresses are reconfigured.
		 */

		getSipxProxyAddresses();

		/*
		 * Start up the STUN address discovery.
		 */
		startAddressDiscovery();

		/*
		 * Register the sip listener with the provider.
		 */
		startSipListener();

		/*
		 * Can start sending outbound calls. Cannot yet gake inbound calls.
		 */
		Gateway.state = GatewayState.INITIALIZED;

		/*
		 * Register with ITSPs. Now we can take inbound calls
		 */
		registerWithItsp();

	}

	/**
	 * Stop the gateway. Release any port resources associated with ongoing
	 * dialogs and tear down ongoing Music oh
	 */
	static synchronized void stop() {
		Gateway.state = GatewayState.STOPPING;
		callControlManager.stop();
		logger.debug("Stopping Gateway");
		// Purge the timer.
		getTimer().purge();
		try {
			for (Dialog dialog : ((SipStackExt) ProtocolObjects.sipStack)
					.getDialogs()) {

				DialogApplicationData dat = DialogApplicationData.get(dialog);

				if (dat != null) {
					BackToBackUserAgent b2bua = dat.getBackToBackUserAgent();
					if (b2bua != null) {
						b2bua.removeDialog(dialog);

					}

					if (dat.getBackToBackUserAgent() != null
							&& dat.getBackToBackUserAgent().getCreatingDialog() == dialog) {

						ItspAccountInfo itspAccountInfo = dat
								.getBackToBackUserAgent().getItspAccountInfo();

						Gateway.decrementCallCount();

						if (itspAccountInfo != null) {
							itspAccountInfo.decrementCallCount();
						}
					}
				}
			}
		} catch (Exception ex) {
			logger.error("Unexepcted exception occured while stopping bridge",
					ex);

		}
		// Tear down the sip stack.
		ProtocolObjects.stop();
		Gateway.state = GatewayState.STOPPED;
	}

	/**
	 * Get the global address of bridge. This is the publicly routable address
	 * of the bridge.
	 * 
	 * @return
	 */
	static String getGlobalAddress() {

		return Gateway.accountManager.getBridgeConfiguration()
				.getGlobalAddress() == null ? globalAddress
				: Gateway.accountManager.getBridgeConfiguration()
						.getGlobalAddress();
	}

	/**
	 * Get the global port of the bridge. This is the publicly routable port of
	 * the bridge.
	 * 
	 * @return
	 */
	static int getGlobalPort() {
		return Gateway.accountManager.getBridgeConfiguration().getGlobalPort() != -1 ? Gateway.accountManager
				.getBridgeConfiguration().getGlobalPort()
				: Gateway.accountManager.getBridgeConfiguration()
						.getExternalPort();
	}

	/**
	 * Get the web server port.
	 * 
	 * @return the web server port.
	 */
	static int getXmlRpcWebServerPort() {
		return Gateway.accountManager.getBridgeConfiguration().getXmlRpcPort();
	}

	/**
	 * Gets the current bridge status.
	 * 
	 * @return
	 */
	static GatewayState getState() {

		return Gateway.state;

	}

	/**
	 * Get the log file name.
	 * 
	 * @return the log file name
	 * 
	 */
	static String getLogFile() {
		return Gateway.getAccountManager().getBridgeConfiguration()
				.getLogFileDirectory()
				+ "/sipxbridge.log";
	}

	/**
	 * Get the default codec name. Returns null if Re-Invite is supported.
	 * 
	 */
	static String getCodecName() {
		if (Gateway.isReInviteSupported())
			return null;
		else
			return Gateway.getAccountManager().getBridgeConfiguration()
					.getCodecName();
	}

	/**
	 * Get the call limit ( number of concurrent calls)
	 */
	static int getCallLimit() {
		return Gateway.getAccountManager().getBridgeConfiguration()
				.getMaxCalls();
	}

	/**
	 * Get the call count.
	 * 
	 * @return -- the call count of the gateway ( number of active calls )
	 * 
	 */
	static int getCallCount() {

		return Gateway.callCount;
	}

	/**
	 * Decrement the call count.
	 */
	static void decrementCallCount() {
		if (Gateway.callCount > 0)
			Gateway.callCount--;
	}

	static void incrementCallCount() {
		Gateway.callCount++;

	}

	/**
	 * Whether or not delayed offer answer model is supported on Re-INVITE
	 * 
	 * @return true if supported.
	 */
	static boolean isReInviteSupported() {

		return accountManager.getBridgeConfiguration().isReInviteSupported();
	}

	static int getSessionExpires() {
		return SESSION_EXPIRES;
	}

	/**
	 * @return the timer
	 */
	static Timer getTimer() {
		return timer;
	}

	/**
	 * Session keepalive timer.
	 * 
	 * @return
	 */
	static String getSessionTimerMethod() {
		return Request.INVITE;
	}

	/**
	 * Returns true if inbound calls go to an auto attendant.
	 * 
	 * @return
	 */
	static boolean isInboundCallsRoutedToAutoAttendant() {
		return accountManager.getBridgeConfiguration()
				.isInboundCallsRoutedToAutoAttendant();
	}

	/**
	 * Place where you want to direct calls ( assuming that you have such a
	 * place ).
	 * 
	 * @return - the address of auto attendant.
	 */
	static String getAutoAttendantName() {
		return accountManager.getBridgeConfiguration().getAutoAttendantName();
	}

	/**
	 * The XML rpc client connection to the symmitron.
	 * 
	 * @param address
	 *            - the address ( extracted from the Via header).
	 * @return -- the client to talk to the symmitron.
	 */
	static SymmitronClient getSymmitronClient(String address) {
		String lookupAddress = address;
		if (Gateway.getBridgeConfiguration().getSymmitronHost() != null) {
			lookupAddress = Gateway.getBridgeConfiguration().getSymmitronHost();
		}

		SymmitronClient symmitronClient = symmitronClients.get(lookupAddress);
		if (symmitronClient == null) {
			symmitronClient = initializeSymmitron(lookupAddress);
			try {
				Thread.sleep(100);
			} catch (Exception ex) {
			}
		}
		return symmitronClient;
	}

	/**
	 * The set of codecs handled by the park server.
	 * 
	 * @return
	 */
	static HashSet<Integer> getParkServerCodecs() {

		return parkServerCodecs;
	}
	
	/**
	 * Log an internal error and potentially throw a runtime exception ( if debug is 
	 * enabled).
	 * 
	 * @param errorString
	 */
    static void logInternalError(String errorString , Exception exception ) {
        if ( logger.isDebugEnabled()) {
            logger.fatal(errorString, exception);
            throw new RuntimeException (errorString, exception);
        }  else {
            logger.fatal(errorString, exception);
        }
        
    }
    
    static void logInternalError(String errorString ) {
        if ( logger.isDebugEnabled()) {
            logger.fatal(errorString);
            throw new RuntimeException (errorString);
        }  else {
            /*
             * Log our stack trace for analysis.
             */
            logger.fatal(errorString, new Exception());
        }
    }


	/**
	 * The main method for the Bridge.
	 * 
	 * @param args
	 */
	public static void main(String[] args) throws Exception {
		try {

			/*
			 * The codecs supported by our park server.
			 */
			parkServerCodecs.add(RtpPayloadTypes.getPayloadType("PCMU"));
			parkServerCodecs.add(RtpPayloadTypes.getPayloadType("PCMA"));

			Gateway.isTlsSupportEnabled = System.getProperty(
					"sipxbridge.enableTls", "false").equals("true");

			Gateway.configurationPath = System.getProperty("conf.dir",
					"/etc/sipxpbx");
			Gateway.configurationFile = System.getProperty("conf.dir",
					"/etc/sipxpbx")
					+ "/sipxbridge.xml";
			String command = System.getProperty("sipxbridge.command", "start");
			String log4jPropertiesFile = Gateway.configurationPath
					+ "/log4j.properties";

			if (command.equals("start")) {
				// Wait for the configuration file to become available.
				while (!new File(Gateway.configurationFile).exists()) {
					Thread.sleep(5 * 1000);
				}
				Gateway.parseConfigurationFile();
				if (new File(log4jPropertiesFile).exists()) {
					/*
					 * Override the file configuration setting.
					 */
					Properties props = new Properties();
					props.load(new FileInputStream(log4jPropertiesFile));
					BridgeConfiguration configuration = Gateway.accountManager
							.getBridgeConfiguration();
					String level = props
							.getProperty("log4j.category.org.sipfoundry.sipxbridge");
					if (level != null) {
						configuration.setLogLevel(level);
					}

				}

				Gateway.initializeLogging();

				Gateway.start();

				startXmlRpcServer();

			} else if (command.equals("configtest")) {
				try {
					if (!new File(Gateway.configurationFile).exists()) {
						System.err
								.println("sipxbridge.xml does not exist - please check configuration.");
						System.exit(-1);
					}
					Gateway.parseConfigurationFile();
					BridgeConfiguration configuration = Gateway.accountManager
							.getBridgeConfiguration();

					if (configuration.getGlobalAddress() == null
							&& configuration.getStunServerAddress() == null) {
						logger
								.error("Configuration error -- no global address or stun server");
						System.err
								.println("sipxbridge.xml: Configuration error: no global address specified and no stun server specified.");
						System.exit(-1);
					}

					if (Gateway.accountManager.getBridgeConfiguration()
							.getExternalAddress().equals(
									Gateway.accountManager
											.getBridgeConfiguration()
											.getLocalAddress())
							&& Gateway.accountManager.getBridgeConfiguration()
									.getExternalPort() == Gateway.accountManager
									.getBridgeConfiguration().getLocalPort()) {
						logger
								.error("Configuration error -- external address == internal address && external port == internal port");
						System.err
								.println("sipxbridge.xml: Configuration error: external address == internal address && external port == internal port");

						System.exit(-1);
					}

					System.exit(0);
				} catch (Exception ex) {
					System.exit(-1);
				}
			} else {
				logger.error("Bad option ");
			}

		} catch (Throwable ex) {
			ex.printStackTrace();
			logger.fatal("Unexpected exception starting", ex);

		}

	}

    
}
